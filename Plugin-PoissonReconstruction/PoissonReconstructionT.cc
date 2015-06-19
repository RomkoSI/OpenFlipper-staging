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

void DumpOutput( const char* format , ... )
{
  va_list args;
  va_start( args , format );
  vprintf( format , args );
  va_end( args );
}

template <class MeshT>
bool
PoissonReconstructionT<MeshT>::
run( std::vector< Real >& _pt_data, MeshT& _mesh, const Parameter& _parameter )
{

    m_parameter = _parameter;

    Real isoValue = 0;

    Octree<2> tree;
#ifdef USE_OMP
    tree.threads = omp_get_num_procs();
#else
    tree.threads = 1;
#endif
    TreeOctNode::SetAllocator( MEMORY_ALLOCATOR_BLOCK_SIZE );

    std::cerr << "Tree construction with depth " << m_parameter.Depth << std::endl;
    tree.setBSplineData( m_parameter.Depth );
    double maxMemoryUsage;
    tree.maxMemoryUsage=0;
    XForm4x4< Real > xForm = XForm4x4< Real >::Identity();
    int pointCount = tree.setTreeMemory( _pt_data ,  m_parameter.Depth ,  m_parameter.MinDepth , m_parameter.Depth , Real(m_parameter.SamplesPerNode),
                                         m_parameter.Scale , m_parameter.Confidence , m_parameter.PointWeight , m_parameter.AdaptiveExponent , xForm );

    std::cerr << "Tree Clipping" << std::endl;

    tree.ClipTree();

    std::cerr << "Tree Finalize" << std::endl;
    tree.finalize( m_parameter.IsoDivide );

    DumpOutput( "Input Points: %d\n" , pointCount );
    DumpOutput( "Leaves/Nodes: %d/%d\n" , tree.tree.leaves() , tree.tree.nodes() );
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage() )/(1<<20) );

    maxMemoryUsage = tree.maxMemoryUsage;
    tree.maxMemoryUsage=0;
    tree.SetLaplacianConstraints();
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage())/(1<<20) );
    maxMemoryUsage = std::max< double >( maxMemoryUsage , tree.maxMemoryUsage );

    tree.maxMemoryUsage=0;
    tree.LaplacianMatrixIteration( m_parameter.SolverDivide, m_parameter.ShowResidual, m_parameter.MinIters, m_parameter.SolverAccuracy, m_parameter.Depth, m_parameter.FixedIters );
    DumpOutput( "Memory Usage: %.3f MB\n" , float( MemoryInfo::Usage() )/(1<<20) );
    maxMemoryUsage = std::max< double >( maxMemoryUsage , tree.maxMemoryUsage );

    CoredFileMeshData mesh;
    if( m_parameter.Verbose ) tree.maxMemoryUsage=0;
    double time=Time();
    isoValue = tree.GetIsoValue();
    DumpOutput( "Got average in: %f\n" , Time()-time );
    DumpOutput( "Iso-Value: %e\n" , isoValue );

    tree.maxMemoryUsage = 0;
    tree.GetMCIsoTriangles( isoValue , m_parameter.IsoDivide , &mesh );

    _mesh.clear();
    mesh.resetIterator();

    DumpOutput( "Time for Iso: %f\n" , Time()-time );











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
