//=============================================================================
//
//  CLASS PoissonReconstructionT - IMPLEMENTATION
//
//=============================================================================

#define ACG_POISSONRECONSTRUCTIONT_C

//== INCLUDES =================================================================

#include "PoissonReconstructionT.hh"


//== NAMESPACES ===============================================================

namespace ACG {

//== IMPLEMENTATION ==========================================================

char* outputFile=NULL;
int echoStdout=0;
void DumpOutput( const char* format , ... )
{
//    if( outputFile )
//    {
//        FILE* fp = fopen( outputFile , "a" );
//        va_list args;
//        va_start( args , format );
//        vfprintf( fp , format , args );
//        fclose( fp );
//        va_end( args );
//    }
//    if( echoStdout )
//    {
        va_list args;
        va_start( args , format );
        vprintf( format , args );
        va_end( args );
//    }
}

void DumpOutput2( char* str , const char* format , ... )
{
//    if( outputFile )
//    {
//        FILE* fp = fopen( outputFile , "a" );
//        va_list args;
//        va_start( args , format );
//        vfprintf( fp , format , args );
//        fclose( fp );
//        va_end( args );
//    }
//    if( echoStdout )
//    {
        va_list args;
        va_start( args , format );
        vprintf( format , args );
        va_end( args );
//    }
//    va_list args;
    va_start( args , format );
    vsprintf( str , format , args );
    va_end( args );
    if( str[strlen(str)-1]=='\n' ) str[strlen(str)-1] = 0;
}


template <class MeshT>
bool
PoissonReconstructionT<MeshT>::
run( std::vector< Real >& _pt_data, MeshT& _mesh, const Parameter& _parameter )
{

    m_parameter = _parameter;

    double t;
//  double tt=Time();
    Real isoValue = 0;

    Octree<2> tree;
#ifdef USE_OMP
    tree.threads = omp_get_num_procs();
#else
    tree.threads = 1;
#endif
    TreeOctNode::SetAllocator( MEMORY_ALLOCATOR_BLOCK_SIZE );

    tree.setBSplineData( m_parameter.Depth );
    double maxMemoryUsage;
    t=Time() , tree.maxMemoryUsage=0;
    XForm4x4< Real > xForm = XForm4x4< Real >::Identity();
    int pointCount = tree.setTreeMemory( _pt_data ,  m_parameter.Depth ,  m_parameter.MinDepth , m_parameter.Depth , Real(m_parameter.SamplesPerNode),
                                         m_parameter.Scale , m_parameter.Confidence , m_parameter.PointWeight , m_parameter.AdaptiveExponent , xForm );
    tree.ClipTree();
    tree.finalize( m_parameter.IsoDivide );

    //    DumpOutput2( comments[commentNum++] , "#             Tree set in: %9.1f (s), %9.1f (MB)\n" , Time()-t , tree.maxMemoryUsage );
    DumpOutput( "Input Points: %d\n" , pointCount );
    DumpOutput( "Leaves/Nodes: %d/%d\n" , tree.tree.leaves() , tree.tree.nodes() );
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage() )/(1<<20) );

    maxMemoryUsage = tree.maxMemoryUsage;
    t=Time() , tree.maxMemoryUsage=0;
    tree.SetLaplacianConstraints();
    //    DumpOutput2( comments[commentNum++] , "#      Constraints set in: %9.1f (s), %9.1f (MB)\n" , Time()-t , tree.maxMemoryUsage );
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage())/(1<<20) );
    maxMemoryUsage = std::max< double >( maxMemoryUsage , tree.maxMemoryUsage );

    t=Time() , tree.maxMemoryUsage=0;
    tree.LaplacianMatrixIteration( m_parameter.SolverDivide, m_parameter.ShowResidual, m_parameter.MinIters, m_parameter.SolverAccuracy, m_parameter.Depth, m_parameter.FixedIters );
    //    DumpOutput2( comments[commentNum++] , "# Linear system solved in: %9.1f (s), %9.1f (MB)\n" , Time()-t , tree.maxMemoryUsage );
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage() )/(1<<20) );
    maxMemoryUsage = std::max< double >( maxMemoryUsage , tree.maxMemoryUsage );

    CoredFileMeshData mesh;
    if( m_parameter.Verbose ) tree.maxMemoryUsage=0;
    t=Time();
    isoValue = tree.GetIsoValue();
    DumpOutput( "Got average in: %f\n" , Time()-t );
    DumpOutput( "Iso-Value: %e\n" , isoValue );

    t = Time() , tree.maxMemoryUsage = 0;
    tree.GetMCIsoTriangles( isoValue , m_parameter.IsoDivide , &mesh );

    _mesh.clear();
    mesh.resetIterator();

    DumpOutput( "Time for Iso: %f\n" , Time()-t );











//    int nr_vertices=int(mesh.outOfCorePointCount()+mesh.inCorePoints.size());
    int nr_faces=mesh.polygonCount();

    mesh.resetIterator();

    //
    // describe vertex and face properties
    //

    // write vertices
    Point3D< float > p;
    for( int i=0 ; i < int( mesh.inCorePoints.size() ) ; i++ )
    {
        p = mesh.inCorePoints[i];
        _mesh.add_vertex( typename MeshT::Point(p[0],p[1],p[2]) );
    }
    for( int i=0; i<mesh.outOfCorePointCount() ; i++ )
    {
        mesh.nextOutOfCorePoint(p);
        _mesh.add_vertex( typename MeshT::Point(p[0],p[1],p[2]) );

    }  // for, write vertices

    // write faces
    std::vector< CoredVertexIndex > polygon;
    for( int i=0 ; i<nr_faces ; i++ )
    {
        //
        // create and fill a struct that the ply code can handle
        //
        mesh.nextPolygon( polygon );
        std::vector< typename MeshT::VertexHandle > face;
        for( int i=0 ; i<int( polygon.size() ) ; i++ )
            if( polygon[i].inCore ) face.push_back( _mesh.vertex_handle( polygon[i].idx ) );
            else                    face.push_back( _mesh.vertex_handle( polygon[i].idx + int( mesh.inCorePoints.size() ) ) );

        _mesh.add_face( face );

    }  // for, write faces

    _mesh.update_normals();

    return true;
}

//-----------------------------------------------------------------------------



//=============================================================================
} // namespace ACG
//=============================================================================
