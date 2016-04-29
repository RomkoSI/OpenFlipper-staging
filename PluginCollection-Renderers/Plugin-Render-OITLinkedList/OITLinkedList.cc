/*===========================================================================*\
*                                                                            *
*                              OpenFlipper                                   *
*      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen       *
*                           www.openflipper.org                              *
*                                                                            *
*--------------------------------------------------------------------------- *
*  This file is part of OpenFlipper.                                         *
*                                                                            *
*  OpenFlipper is free software: you can redistribute it and/or modify       *
*  it under the terms of the GNU Lesser General Public License as            *
*  published by the Free Software Foundation, either version 3 of            *
*  the License, or (at your option) any later version with the               *
*  following exceptions:                                                     *
*                                                                            *
*  If other files instantiate templates or use macros                        *
*  or inline functions from this file, or you compile this file and          *
*  link it with other files to produce an executable, this file does         *
*  not by itself cause the resulting executable to be covered by the         *
*  GNU Lesser General Public License. This exception does not however        *
*  invalidate any other reasons why the executable file might be             *
*  covered by the GNU Lesser General Public License.                         *
*                                                                            *
*  OpenFlipper is distributed in the hope that it will be useful,            *
*  but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*  GNU Lesser General Public License for more details.                       *
*                                                                            *
*  You should have received a copy of the GNU LesserGeneral Public           *
*  License along with OpenFlipper. If not,                                   *
*  see <http://www.gnu.org/licenses/>.                                       *
*                                                                            *
\*===========================================================================*/

/*===========================================================================*\
*                                                                            *
*   $Revision$                                                       *
*   $LastChangedBy$                                                *
*   $Date$                     *
*                                                                            *
\*===========================================================================*/

#include <ACG/GL/gl_compat_4_4.hh>

#include "OITLinkedList.hh"


#include <OpenFlipper/common/GlobalOptions.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>

#include <ACG/GL/ShaderCache.hh>
#include <ACG/GL/ScreenQuad.hh>

#include <ACG/GL/GLError.hh>



/*
OIT with per pixel linked list (A-Buffer).

Implementation based on AMD presentation at GDC 2010:
 "OIT and GI using DX11 linked lists" by Nick Thibieroz & Holger Gr�n


porting from DX11 to GL:

UAV : image load/store  with glBindImageTexture()

[earlydepthstencil] : layout(early_fragment_tests) in;

UAV counter: atomic counter


glsl   gpu_shader5:

convert between float4 color and uint r8g8b8a8:   packUnorm4x8(color)         unpackUnorm4x8(r8g8b8a8)
reinterpret cast between float and int:  i_val = floatBitsToInt(f_val)        intBitsToFloat(i_val)



msaa support:
SV_Coverage : gl_SampleMaskIn[0],    gl_SampleMask[0]
SV_SampleIndex :  gl_SampleID

msaa implementation: two different methods tested
 - shading per sample via gl_SampleID
 - shading per pixel and manually resolve sample colors

 Generation of the ABuffer is problematic in msaa mode, as it seems to run per-sample as indicated by the atomic counter value.
 Although per-sample and per-fragment give the same result, per-fragment shading is preferred to save gpu ram.

 Forcing per fragment via glDisable(GL_MULTISAMPLING) will give a coverage mask of 0x01,
  and only a single sample gets shaded and the others stay black..

 glDisable(GL_SAMPLE_SHADING) and glMinSampleShading(0.0f) still results in per sample shading
*/

// =================================================

// generation of per pixel linked lists (ABuffer)
class ABufferGenModifier : public ACG::ShaderModifier
{
public:

  void modifyFragmentIO(ACG::ShaderGenerator* _shader)
  {
    _shader->setGLSLVersion(420);


    _shader->addUniform("layout(binding = 0, offset = 0) uniform atomic_uint g_FragmentCounter");
    _shader->addUniform("layout(binding = 0, r32ui)  uniform uimageBuffer g_StartOffsetBuffer");
    _shader->addUniform("layout(binding = 1, rgba32ui)  uniform uimageBuffer g_ABuffer");


    _shader->addUniform("uniform uvec2 g_ScreenSize");
  }

  void modifyFragmentBeginCode(QStringList* _code)
  {
  }

  void modifyFragmentEndCode(QStringList* _code)
  {
    // convert color from float4 to uint
    _code->push_back("uint uiColor = packUnorm4x8(outFragment);");

    // convert float depth to uint
    _code->push_back("uint uiDepth = floatBitsToUint(gl_FragCoord.z);");
    // alternative depth [0,1] to uint cast:
    // uiDepth = depth * (2^32-1)

    // get last fragment id
    _code->push_back("uint uiFragCount = atomicCounterIncrement(g_FragmentCounter);");

    // get pixel index of current fragment
    _code->push_back("uvec2 absFragCoord = uvec2(gl_FragCoord.xy);");
    _code->push_back("uint uiPixelID = absFragCoord.x + uint(g_ScreenSize.x) * absFragCoord.y;");

    // take current offset from StartOffsetBuffer for fragment and replace it with the current fragment id
    //  this builds a reverse linked list (last inserted fragment is the head of the list of the current pixel)
    // note: this operation doesnt even have to be atomic (the counter is atomic already), so optimization could be possible
    _code->push_back("uint uiNextFragment = imageAtomicExchange(g_StartOffsetBuffer, int(uiPixelID), uiFragCount);");

    // append fragment entry at end of ABuffer, each fragment stores: (color, depth, nextEntry, msaaCoverageMask)
    _code->push_back("imageStore(g_ABuffer, int(uiFragCount), uvec4(uiColor, uiDepth, uiNextFragment, uint(gl_SampleMaskIn[0])) );");


    // in this pass we write into the UAV buffers only
    //  transparent objects should not change the depth buffer
    //  color is resolved in a post-processing step
//    _code->push_back("discard;");
  }

  static ABufferGenModifier instance;
};

ABufferGenModifier ABufferGenModifier::instance;

OITLinkedList::OITLinkedList()
{
}


OITLinkedList::~OITLinkedList()
{
}


void OITLinkedList::initializePlugin()
{
  ACG::ShaderProgGenerator::setShaderDir(OpenFlipper::Options::shaderDirStr());
  ACG::ShaderProgGenerator::registerModifier(&ABufferGenModifier::instance);
}



void OITLinkedList::render(ACG::GLState* _glState, Viewer::ViewerProperties& _properties)
{
  // collect renderobjects + prepare OpenGL state
  prepareRenderingPipeline(_glState, _properties.drawMode(), PluginFunctions::getSceneGraphRootNode());


  renderOIT(_glState->viewport_width(), _glState->viewport_height(), _properties.multisampling());

  // restore common opengl state
  // log window remains hidden otherwise
  finishRenderingPipeline();
}

void OITLinkedList::prepareBuffers(int w, int h)
{
  ACG::Vec2ui screenSize = ACG::Vec2ui(GLuint(w), GLuint(h));

  // prepare buffers
  const int estAvgLayers = 3;

  const int numPixels = w * h;
  const int offsetBufSize = numPixels * 4; // uint buffer for each pixel
  const int ABufSize = numPixels * 16 * estAvgLayers;

  if (startOffsetBuffer_.getBufferSize() < offsetBufSize)
    startOffsetBuffer_.setBufferData(offsetBufSize, 0, GL_R32UI, GL_DYNAMIC_DRAW);

  if (ABuffer_.getBufferSize() < ABufSize)
    ABuffer_.setBufferData(ABufSize, 0, GL_RGBA32UI, GL_DYNAMIC_DRAW);


  // dbg: force unbind
  glBindTexture(GL_TEXTURE_BUFFER, 0);

  // reset fragment counter
  fragCounter_.set(0);

  // wait for buffer allocation
  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void OITLinkedList::renderOIT(int w, int h, bool multisampled)
{
  ACG::Vec2ui screenSize = ACG::Vec2ui(GLuint(w), GLuint(h));

  prepareBuffers(w, h);


  // ----------------------------------
  // invalidate per-pixel linked lists ( reset startOffsetBuffer to 0xffffffff )
  glColorMask(0, 0, 0, 0);
  glDepthMask(0);
  glDisable(GL_DEPTH_TEST);

  // load reset shader for startOffsetBuffer
  GLSL::Program* shaderReset = ACG::ShaderCache::getInstance()->getProgram("ScreenQuad/screenquad.glsl", "OITLinkedList/reset.glsl");

  startOffsetBuffer_.bindAsImage(0, GL_WRITE_ONLY);

  shaderReset->use();

  shaderReset->setUniform("g_ScreenSize", screenSize);

  // screenquad scale and offset
  shaderReset->setUniform("size", ACG::Vec2f(1.0f, 1.0f));
  shaderReset->setUniform("offset", ACG::Vec2f(0.0f, 0.0f));

  ACG::ScreenQuad::draw(shaderReset);

  // wait for atomic counter and reset pass to finish!
  glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

  // ----------------------------------
  // 1. pass
  // render scene into A-Buffer

  // msaa: per pixel lists instead of per sample lists
  //       note: shader seems to be run per sample anyway 
  //         (ati driver: AtomicCounter with msaa enabled is a lot higher than without for the same scene)
  if (multisampled)
  {
    glDisable(GL_SAMPLE_SHADING);
    glEnable(GL_MULTISAMPLE);
    glMinSampleShading(0.0f);
  }

  int msaaSampleCount = 1;
  glGetIntegerv(GL_SAMPLES, &msaaSampleCount);

  // bind UAV buffers
  startOffsetBuffer_.bindAsImage(0, GL_READ_WRITE);
  ABuffer_.bindAsImage(1, GL_WRITE_ONLY);

  // bind atomic counter
  fragCounter_.bind(0);

  for (int i = 0; i < getNumRenderObjects(); ++i)
  {
    // apply ABufferGenModifier to all objects
    ACG::RenderObject* obj = getRenderObject(i);

    GLSL::Program* shaderRender = ACG::ShaderCache::getInstance()->getProgram(&obj->shaderDesc, ABufferGenModifier::instance);

    shaderRender->use();
    shaderRender->setUniform("g_ScreenSize", screenSize);

    // disable writing to back-buffer
    // only write linked lists in A-Buffer
    obj->glColorMask(0,0,0,0);
    obj->depthTest = false;
    obj->depthWrite = false;
    obj->culling = false;

    renderObject(obj, shaderRender);
  }

  fragCounter_.unbind();


  // wait for render pass to finish!
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);

  // ----------------------------------
  // 2. pass
  // resolve A-Buffer
  glColorMask(1, 1, 1, 1);
  glDepthMask(1);

  // msaa: resolve lists per sample
  if (multisampled)
  {
    // force per sample shading
    glEnable(GL_SAMPLE_SHADING);
    glEnable(GL_MULTISAMPLE);
    glMinSampleShading(1.0f);
  }

  glGetIntegerv(GL_SAMPLES, &msaaSampleCount);


  glDepthFunc(GL_ALWAYS);


  // load resolve shader from cache
  GLSL::Program* shaderResolve = 0;
  
  // msaa is enabled via preprocessor macro in the shader
  QStringList shaderMacros;
  if (multisampled)
    shaderMacros.push_back("#define OITLL_MSAA");

  shaderResolve = ACG::ShaderCache::getInstance()->getProgram("ScreenQuad/screenquad.glsl", "OITLinkedList/resolve.glsl", &shaderMacros);


  // init shader constants
  shaderResolve->use();

  startOffsetBuffer_.bindAsImage(0, GL_READ_ONLY);
  ABuffer_.bindAsImage(1, GL_READ_ONLY);

  shaderResolve->setUniform("g_ScreenSize", screenSize);

  if (multisampled)
    shaderResolve->setUniform("g_SampleCount", GLuint(msaaSampleCount));

  // screenquad scale and offset
  shaderResolve->setUniform("size", ACG::Vec2f(1.0f, 1.0f));
  shaderResolve->setUniform("offset", ACG::Vec2f(0.0f, 0.0f));
  

  ACG::ScreenQuad::draw(shaderResolve);


  // wait for resolve pass
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);



  if (multisampled)
  {
    glDisable(GL_SAMPLE_SHADING);
    glMinSampleShading(0.0f);
  }

  // unbind

  const int numBoundImages = 3;

  for (int i = 0; i < numBoundImages; ++i)
    glBindImageTexture(i, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

  // -----------------------
  // get result of fragment counter, ie. the total fragment count of the scene

  unsigned int actualFragmentCount = 0;

  fragCounter_.get(&actualFragmentCount);


  // resize ABuffer if too small
  // RGBA32UI : 16 bytes per fragment
  if (int(actualFragmentCount) * 16 > ABuffer_.getBufferSize())
  {
    ABuffer_.setBufferData(actualFragmentCount * 18, 0, ABuffer_.getFormat(), ABuffer_.getUsage());

//    std::cout << "buffer size too small: " << actualFragmentCount << std::endl;
  }
}



QString OITLinkedList::checkOpenGL()
{
  // Get version and check
  if ( !ACG::openGLVersion(4,2) )
    return QString("Insufficient OpenGL Version! OpenGL 4.2 or higher required");

  // Check extensions
  QString glExtensions = QString((const char*)glGetString(GL_EXTENSIONS));
  QString missing("");
  if ( !glExtensions.contains("ARB_shader_image_load_store") )
    missing += "ARB_shader_image_load_store extension missing\n";

  if ( !glExtensions.contains("GL_ARB_shader_atomic_counters") )
    missing += "GL_ARB_shader_atomic_counters extension missing\n";

  if ( !glExtensions.contains("GL_ARB_gpu_shader5") )
    missing += "GL_ARB_gpu_shader5 extension missing\n";

  return missing;
}


QString OITLinkedList::renderObjectsInfo( bool _outputShaderInfo )
{
  std::vector<ACG::ShaderModifier*> mods;
  mods.push_back(&ABufferGenModifier::instance);
  return dumpCurrentRenderObjectsToString(&sortedObjects_[0],_outputShaderInfo, &mods);
}

#if QT_VERSION < 0x050000
  Q_EXPORT_PLUGIN2( oitlinkedlist , OITLinkedList );
#endif



