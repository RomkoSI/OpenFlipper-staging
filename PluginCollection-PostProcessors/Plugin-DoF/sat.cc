/*===========================================================================*\
*                                                                            *
*                              OpenFlipper                                   *
*      Copyright (C) 2001-2011 by Computer Graphics Group, RWTH Aachen       *
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
*   $Revision: 17080 $                                                       *
*   $LastChangedBy: moeller $                                                *
*   $Date: 2013-07-19 12:58:31 +0200 (Fri, 19 Jul 2013) $                     *
*                                                                            *
\*===========================================================================*/

#include <ACG/GL/gl_compat_4_4.hh>

#include "sat.hh"

#include <ACG/GL/ShaderCache.hh>
#include <ACG/ShaderUtils/GLSLShader.hh>
#include <ACG/GL/GLFormatInfo.hh>

//#include <ACG/GL/Debug.hh>
//#define SAT_DBG_DIR "/home/tenter/dbg/sat_out/"
#define SAT_DBG_DIR "c:/dbg/sat_out/"

//#define SAT_DBG


/*
implementation reference:
"Parallel Prefix Sum (Scans) with CUDA" by M. Harris et. al.,  GPU Gems 3
https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch39.html
*/


std::map<GLenum, const char*> PrefixSumPlan::datatypeMacros_;

PrefixSumPlan::PrefixSumPlan(int _w, int _h, GLenum _internalFmt, int _blocksize)
  : width_(_w), height_(_h), blocksize_(_blocksize), internalFmt_(_internalFmt), elemSize_(0),
  blockSumPlan_(0),
  dbgOutput_(0), dbgTranposedInput_(0), dbgProfile_(0)
{
  // blocksize must be power of 2
  _blocksize = paddedBlocksize(_blocksize);

  // padding should not required, but uncommenting this enables automatic padding
//  width_ = _w = paddedDimension(_w);

  numWorkGroupsX_ = _w  / ( _blocksize) + (_w % _blocksize ? 1 : 0);
  numWorkGroupsY_ = _h;

  numWorkGroupsX_ = std::max(numWorkGroupsX_, 1);
  numWorkGroupsY_ = std::max(numWorkGroupsY_, 1);

  numBlockScanGroupsX_ = numWorkGroupsX_ / 2 + numWorkGroupsX_ % 2;
  numBlockScanGroupsY_ = numWorkGroupsY_ / 2 + numWorkGroupsY_ % 2;

  numBlockScanGroupsX_ = std::max(numBlockScanGroupsX_, 1);
  numBlockScanGroupsY_ = std::max(numBlockScanGroupsY_, 1);

  numDispatches_ = 1;

  // configure shader via preprocessor defines

  QString blocksizeDef;
  blocksizeDef.sprintf("#define SAT_BLOCKSIZE %i", _blocksize);
  macros_.push_back(blocksizeDef);

  if (_h > 1)
    macros_.push_back("#define SAT_2D");

  if (datatypeMacros_.empty())
  {
    datatypeMacros_[GL_R32F] = "SAT_FLOAT1";
    datatypeMacros_[GL_RG32F] = "SAT_FLOAT2";
    datatypeMacros_[GL_RGBA32F] = "SAT_FLOAT4";

    datatypeMacros_[GL_R32I] = "SAT_INT1";
    datatypeMacros_[GL_RG32I] = "SAT_INT2";
    datatypeMacros_[GL_RGBA32I] = "SAT_INT4";

    datatypeMacros_[GL_R32UI] = "SAT_UINT1";
    datatypeMacros_[GL_RG32UI] = "SAT_UINT2";
    datatypeMacros_[GL_RGBA32UI] = "SAT_UINT4";
  }

  std::map<GLenum, const char*>::iterator datatype = datatypeMacros_.find(_internalFmt);

  if (datatype != datatypeMacros_.end())
  {
    macros_.push_back(QString("#define SAT_DATATYPE ") + QString(datatype->second));

    bool success = false;
    elemSize_ = 4 * QString(datatype->second).right(1).toInt(&success);

    if (!success)
      std::cout << "SATPlan: failed to get size of format " << datatype->second << std::endl;
  }
  else
    std::cout << "SATPlan: unsupported texture format " << _internalFmt << std::endl;


  // requires blocksum reduction chain
  if (numWorkGroupsX_ > 1)
  {
    macros_.push_back("#define SAT_BLOCKSCANOUT");
    blockSumPlan_ = new PrefixSumPlan(numWorkGroupsX_, height_, _internalFmt, _blocksize);
  }
}

PrefixSumPlan::~PrefixSumPlan()
{
}

int PrefixSumPlan::paddedDimension(int _dim) const
{
  int padding = _dim % (2*blocksize_);
  if (padding)
    return _dim + 2*blocksize_ - padding;
  return _dim;
}

int PrefixSumPlan::paddedBlocksize( int _size ) const
{
  int i = 1; 
  while (i < _size) i <<= 1;
  return i;
}

bool PrefixSumPlan::execute( ACG::TextureBuffer* _src, ACG::TextureBuffer* _dst )
{
  bool success = true;

  GLSL::Program* satCS = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/psum.glsl", &macros_);

  if (satCS)
  {
    if (dbgProfile_)
      perfCounter_.restart();

    _src->bindAsImage(0, GL_READ_ONLY);
    _dst->bindAsImage(1, GL_WRITE_ONLY);

    if (numWorkGroupsX_ > 1)
    {
      if (!blockSums_.is_valid())
      {
        blockSums_.setBufferData(numBlockScanGroupsX_ * 2 * elemSize_, 0, internalFmt_);
        blockSumsOut_.setBufferData(numBlockScanGroupsX_ * 2 * elemSize_, 0, internalFmt_);
      }
      blockSums_.bindAsImage(2, GL_WRITE_ONLY);
    }


    satCS->use();

    glDispatchCompute(numWorkGroupsX_,numWorkGroupsY_,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    if (dbgProfile_)
      std::cout << "psum: " << perfCounter_.elapsedMs() << std::endl;

#ifdef SAT_DBG
    if (dbgOutput_)
      ACG::GLDebug::dumpBufferData(GL_TEXTURE_BUFFER, _dst->getBufferId(), SAT_DBG_DIR "1d_pass1.bin");
#endif

    if (numWorkGroupsX_ > 1)
    {
      if (blockSumPlan_)
      {
        if (dbgProfile_)
          perfCounter_.restart();

        blockSumPlan_->execute(&blockSums_, &blockSumsOut_);

        if (dbgProfile_)
          std::cout << "psum-blockscan: " << perfCounter_.elapsedMs() << std::endl;


        GLSL::Program* satCSBlockMerge = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/psum_blockmerge.glsl", &macros_);


        if (satCSBlockMerge)
        {
          if (dbgProfile_)
            perfCounter_.restart();

          _dst->bindAsImage(0, GL_READ_WRITE);
          blockSumsOut_.bindAsImage(1, GL_READ_ONLY);

          satCSBlockMerge->use();
          glDispatchCompute(numWorkGroupsX_-1,1,1);
          glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

          if (dbgProfile_)
            std::cout << "psum-blockmerge: " << perfCounter_.elapsedMs() << std::endl;

          success = true;
        }
      }
    }
    else
      success = true;

#ifdef SAT_DBG
    if (dbgOutput_)
    {
      ACG::GLDebug::dumpBufferData(GL_TEXTURE_BUFFER, _dst->getBufferId(), SAT_DBG_DIR "1d_act.bin");
      if (blockSumsOut_.is_valid())
        ACG::GLDebug::dumpBufferData(GL_TEXTURE_BUFFER, blockSumsOut_.getBufferId(), SAT_DBG_DIR "1d_bsum.bin");
    }
#endif
  }

  return success;
}


bool PrefixSumPlan::execute( ACG::Texture2D* _src, ACG::Texture2D* _dst )
{
  bool success = false;

  GLSL::Program* satCS = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/psum.glsl", &macros_);

#ifdef SAT_DBG
  if (dbgOutput_)
    ACG::GLDebug::dumpTexture2D(_src->id(), 0, _src->getFormat(), _src->getType(), elemSize_ * _src->getWidth() * _src->getHeight(),  QString(QString(SAT_DBG_DIR "2d_") + QString(dbgTranposedInput_ ? "cols_" : "rows_") + QString("input.bin")).toLatin1(), true);
#endif

  if (satCS)
  {
    // 1. divide: compute local prefixsums in each block of each row

    if (dbgProfile_)
      perfCounter_.restart();

    _src->bindAsImage(0, GL_READ_ONLY);
    _dst->bindAsImage(1, GL_WRITE_ONLY);

    if (numWorkGroupsX_ > 1)
    {
      if (!blockSums2D_.is_valid())
      {
        blockSums2D_.setStorage(1, internalFmt_, 2 * numBlockScanGroupsX_, height_);
        blockSums2D_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        blockSums2D_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        blockSums2DOut_.setStorage(1, internalFmt_, 2 * numBlockScanGroupsX_, height_);
        blockSums2DOut_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        blockSums2DOut_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      }

      blockSums2D_.bindAsImage(2, GL_WRITE_ONLY);
    }

    satCS->use();

    glDispatchCompute(numWorkGroupsX_,height_,1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    if (dbgProfile_)
      std::cout << "psum-blockscan: " << perfCounter_.elapsedMs() << std::endl;

#ifdef SAT_DBG
    if (dbgOutput_)
      ACG::GLDebug::dumpTexture2D(_dst->id(), 0, _dst->getFormat(), _dst->getType(), elemSize_ * _dst->getWidth() * _dst->getHeight(), (QString(SAT_DBG_DIR "2d_pass1") + QString(dbgTranposedInput_? "_cols" : "_rows") + QString(".bin")).toLatin1(), true);
#endif

    if (numWorkGroupsX_ > 1)
    {
      if (blockSumPlan_)
      {
        // 2. blockscan: compute prefixsum of the blocksums in each row

        if (dbgProfile_)
          perfCounter_.restart();

        blockSumPlan_->execute(&blockSums2D_, &blockSums2DOut_);

        if (dbgProfile_)
          std::cout << "psum-blockscan: " << perfCounter_.elapsedMs() << std::endl;

        GLSL::Program* satCSBlockMerge = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/psum_blockmerge.glsl", &macros_);


        if (satCSBlockMerge)
        {
          // 3. blockmerge: add prefixsum of the blocksums to the local prefixsums in the row blocks

          if (dbgProfile_)
            perfCounter_.restart();

          _dst->bindAsImage(0, GL_READ_WRITE);
          blockSums2DOut_.bindAsImage(1, GL_READ_ONLY);

          satCSBlockMerge->use();
          glDispatchCompute(numWorkGroupsX_-1,height_,1);
          glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

          if (dbgProfile_)
            std::cout << "psum-blockmerge: " << perfCounter_.elapsedMs() << std::endl;

          success = true;
        }
      }
    }
    else 
      success = true;

    // now, each row contains the prefixsums of the rows in the input image

#ifdef SAT_DBG
    if (dbgOutput_)
    {
      ACG::GLDebug::dumpTexture2D(_dst->id(), 0, _dst->getFormat(), _dst->getType(), elemSize_ * _dst->getWidth() * _dst->getHeight(),  QString(QString(SAT_DBG_DIR "2d_")  + QString(dbgTranposedInput_? "_cols" : "_rows") + QString(".bin")).toLatin1(), true);
      if (blockSums2DOut_.is_valid())
        ACG::GLDebug::dumpTexture2D(blockSums2DOut_.id(), 0, blockSums2DOut_.getFormat(), blockSums2DOut_.getType(), elemSize_ * width_ * height_,  QString(QString(SAT_DBG_DIR "2d_")  + QString(dbgTranposedInput_? "_cols" : "_rows") + QString("_bsum.bin")).toLatin1(), true);
    }
#endif
  }

  return success;
}



bool PrefixSumPlan::padInput( ACG::Texture2D* _src, ACG::Texture2D* _dst, bool _padWidthAndHeight )
{
  bool success = false;

  GLSL::Program* padCS = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/pad.glsl", &macros_);

  if (padCS)
  {
    padCS->use();

    _src->bindAsImage(0, GL_READ_ONLY);

    int padHeight = height_;
    if (_padWidthAndHeight)
      padHeight = paddedDimension(padHeight);

    if (!_dst->is_valid())
    {
      _dst->setStorage(1, internalFmt_, width_, padHeight);
      _dst->parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      _dst->parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    _dst->bindAsImage(1, GL_WRITE_ONLY);

    glDispatchCompute(numWorkGroupsX_, padHeight / blocksize_, 1);

    success = true;
  }

  return success;
}




SATPlan::SATPlan( int _w, int _h, GLenum _internalFmt /*= GL_R32F*/, int _blocksize /*= 32*/ )
  : rows_(0), cols_(0), paddingRequired_(false), transposeGroupSize_(0)
{
  rows_ = new PrefixSumPlan(_w, _h, _internalFmt, _blocksize);
  cols_ = new PrefixSumPlan(_h, _w, _internalFmt, _blocksize);

  paddingRequired_ = _w != rows_->width() || _h != cols_->width();

  rows_->debugSetTransposedInput(0);
  cols_->debugSetTransposedInput(1);

  transposeMacros_ = rows_->macros();


  transposeGroupSize_ = _blocksize;
  int maxInvocations = GLSL::ComputeShader::caps().maxWorkGroupInvocations_;

  if (transposeGroupSize_*transposeGroupSize_ > maxInvocations)
  {
    transposeGroupSize_ = int(sqrtf(float(maxInvocations)));

    QString cappedBlocksize;
    cappedBlocksize.sprintf("#define SAT_BLOCKSIZE %d", transposeGroupSize_);

    transposeMacros_.push_back("#undef SAT_BLOCKSIZE");
    transposeMacros_.push_back(cappedBlocksize);
  }
}

SATPlan::~SATPlan()
{
  delete rows_;
  delete cols_;
}

bool SATPlan::transpose( ACG::Texture2D* _src, ACG::Texture2D* _dst )
{
  GLSL::Program* transposeCS = ACG::ShaderCache::getInstance()->getComputeProgram("SAT/transpose.glsl", &transposeMacros_);
  
  if (transposeCS)
  {
    if (rows_->profilingEnabled())
      perfCounter_.restart();

    _src->bindAsImage(0, GL_READ_ONLY);
    _dst->bindAsImage(1, GL_WRITE_ONLY);

    int w = _src->getWidth();
    int h = _src->getHeight();
    int blocksize = transposeGroupSize_;

    int numGroupsX = w / blocksize + (w % blocksize ? 1 : 0);
    int numGroupsY = h / blocksize + (h % blocksize ? 1 : 0);

    transposeCS->use();
    glDispatchCompute(numGroupsX, numGroupsY, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    if (rows_->profilingEnabled())
      std::cout << "transpose: " << perfCounter_.elapsedMs() << std::endl;
  }

  return transposeCS != 0;
}

bool SATPlan::execute( ACG::Texture2D* _src, ACG::Texture2D* _dst )
{
  // SAT via separable prefixsum filters


  if (paddingRequired_)
  {
    if (rows_->profilingEnabled())
     perfCounter_.restart();

    if (!paddedInput_.is_valid())
    {
      paddedInput_.setStorage(1, _src->getInternalFormat(), rows_->width(), cols_->width());
      paddedInput_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      paddedInput_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    rows_->padInput(_src, &paddedInput_, true);

    if (rows_->profilingEnabled())
      std::cout << "padding: " << perfCounter_.elapsedMs() << std::endl;
  }


  if (rows_->profilingEnabled())
    perfCounter_.restart();

  // prefixsum of rows
  bool success = rows_->execute(paddingRequired_ ? &paddedInput_ : _src, _dst);

  if (rows_->profilingEnabled())
    std::cout << "rows: " << perfCounter_.elapsedMs() << std::endl;


  if (success)
  {
    if (!transposedSrc_.is_valid())
    {
      transposedSrc_.setStorage(1, _src->getInternalFormat(), cols_->width(), cols_->height());
      transposedSrc_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      transposedSrc_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    if (!transposedDst_.is_valid())
    {
      transposedDst_.setStorage(1, _src->getInternalFormat(), cols_->width(), cols_->height());
      transposedDst_.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      transposedDst_.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }

    // transpose
    success = transpose(_dst, &transposedSrc_);

#ifdef SAT_DBG
    if (rows_->debugOutputEnabled())
      ACG::GLDebug::dumpTexture2D(transposedSrc_.id(), 0, transposedSrc_.getFormat(), transposedSrc_.getType(), rows_->elemSize() * transposedSrc_.getWidth() * transposedSrc_.getHeight(),  SAT_DBG_DIR "2d_rows_tr.bin", true);
#endif

    if (success)
    {
      if (rows_->profilingEnabled())
        perfCounter_.restart();

      // prefixsum of cols to compute SAT
      success = cols_->execute(&transposedSrc_, &transposedDst_);

      if (rows_->profilingEnabled())
        std::cout << "cols: " << perfCounter_.elapsedMs() << std::endl;

#ifdef SAT_DBG
      if (rows_->debugOutputEnabled())
        ACG::GLDebug::dumpTexture2D(transposedDst_.id(), 0, transposedSrc_.getFormat(), transposedSrc_.getType(), rows_->elemSize() * transposedSrc_.getWidth() * transposedSrc_.getHeight(),  SAT_DBG_DIR "2d_sat_tr.bin", true);
#endif

      if (success)
      {
        // transpose back
        success = transpose(&transposedDst_, _dst);

#ifdef SAT_DBG
        if (rows_->debugOutputEnabled())
          ACG::GLDebug::dumpTexture2D(_dst->id(), 0, transposedSrc_.getFormat(), transposedSrc_.getType(), rows_->elemSize() * transposedSrc_.getWidth() * transposedSrc_.getHeight(),  SAT_DBG_DIR "2d_sat.bin", true);
#endif
      }
    }
  }

  return success;
}

void SATPlan::enableDebugOutput()
{
  rows_->enableDebugOutput();
  cols_->enableDebugOutput();
}

void SATPlan::enableProfiling()
{
  rows_->enableProfiling();
  cols_->enableProfiling();
}







bool PrefixSumPlan::testBuffer(int w, int cmpMem, int fullOutput)
{
  ACG::TextureBuffer testBuffer, testBufferOut;

  int numTestVals = w;
  int blockSize = 32;
  int numBlocks = numTestVals / blockSize;
  GLenum internalfmt = GL_R32I;
  int elemDim = ACG::GLFormatInfo(internalfmt).channelCount();

  std::vector<int> testData(numTestVals * elemDim);

  for (int i = 0; i < numTestVals; ++i)
  {
    testData[i*elemDim] = i;
    if (elemDim > 1)
      testData[i*elemDim+1] = i+1;
    if (elemDim > 2)
      testData[i*elemDim+2] = -i;
    if (elemDim > 3)
      testData[i*elemDim+3] = i*2;
  }

  testBuffer.setBufferData(testData.size()*sizeof(testData[0]), &testData[0], internalfmt);
  testBufferOut.setBufferData(testData.size()*sizeof(testData[0]), &testData[0], internalfmt);

  // compute expected result
  std::vector<int> expectedData(numTestVals * elemDim, 0);

  for (int i = 1; i < numTestVals; ++i)
  {
    for (int k = 0; k < elemDim; ++k)
      expectedData[i*elemDim + k] = testData[(i-1)*elemDim + k] + expectedData[(i-1)*elemDim + k];
  }

  


  PrefixSumPlan plan(numTestVals, 1, internalfmt, blockSize);
  if (fullOutput)
    plan.enableDebugOutput();
  plan.execute(&testBuffer, &testBufferOut);


  bool success = true;

  if (cmpMem)
  {
    int bufSize = numTestVals * elemDim * 4;
    char* pActBuf = new char[bufSize];
    testBufferOut.getBufferData(pActBuf);

    if (memcmp(pActBuf, &expectedData[0], bufSize))
    {
      printf("error for %d\n", w);
      success = false;
    }

    if (!success)
    {
      char szFileOut[0xff];
      sprintf(szFileOut, SAT_DBG_DIR "1d_act_%03d.bin", numTestVals);
      FILE* pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        fwrite(pActBuf, 1, bufSize, pFile);
        fclose(pFile);
      }

      sprintf(szFileOut, SAT_DBG_DIR "1d_in_%03d.bin", numTestVals);
      pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        fwrite(&testData[0], sizeof(testData[0]), testData.size(), pFile);
        fclose(pFile);
      }

      sprintf(szFileOut, SAT_DBG_DIR "1d_exp_%03d.bin", numTestVals);
      pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        fwrite(&expectedData[0], sizeof(expectedData[0]), expectedData.size(), pFile);
        fclose(pFile);
      }

      sprintf(szFileOut, SAT_DBG_DIR "1d_expbsum_%03d.bin", numTestVals);
      pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        int bsum[4] = {0};

        for (int i = 1; i < numBlocks; ++i)
        {
          memset(bsum, 0, sizeof(bsum));

          for (int e = 0; e < elemDim; ++e)
          {
            int shift = 0;

//            if (i > 1)
              shift = expectedData[((i-1)*blockSize) * elemDim + e];

            bsum[e] = expectedData[(i*blockSize) * elemDim + e] - shift;
          }
//            bsum[e] += testData[offset + k * elemDim + e];

          fwrite(bsum, 4, elemDim, pFile);

//           if (i > 0)
//           {
//             int offset = i * blockSize * elemDim;
// 
//             for (int k = 0; k < blockSize; ++k)
//             {
//               for (int e = 0; e < elemDim; ++e)
//                 bsum[e] += testData[offset + k * elemDim + e];
//             }
//           }
        }
        fclose(pFile);
      }
    }

    delete [] pActBuf;
  }

  return success;
}



bool PrefixSumPlan::test2D( int w, int h, int cmpMem, int fullOutput )
{
  int numTestVals = w*h;
  int blockSize = 32;
  int numBlocks = numTestVals / blockSize;
  GLenum internalfmt = GL_R32I;
  ACG::GLFormatInfo finfo(internalfmt);
  int elemDim = finfo.channelCount();

  std::vector<int> testData(numTestVals);
  for (int i = 0; i < numTestVals; ++i)
    testData[i] = i;

//   std::vector<ACG::Vec2i> testData(numTestVals);
//   for (int i = 0; i < numTestVals; ++i)
//     testData[i] = ACG::Vec2i(i,i+1);

  // compute expected result
  std::vector<int> expectedData = testData;
//  std::vector<ACG::Vec2i> expectedData = testData;
  PrefixSumPlan::executeRowsCPU(w,h, expectedData);


  char szFileOut[0xff];
//   sprintf(szFileOut, SAT_DBG_DIR "2d_in_%dx%d.bin", w,h);
//   FILE* pFile = fopen(szFileOut, "wb");
//   if (pFile)
//   {
//     fwrite(&testData[0], sizeof(testData[0]), testData.size(), pFile);
//     fclose(pFile);
//   }
// 
// 


  PrefixSumPlan::executeColsCPU(w,h,expectedData);




  ACG::Texture2D testBuffer, testBufferOut;
  testBuffer.setData(0, internalfmt, w, h, finfo.format(), finfo.type(), &testData[0]);
//  testBuffer.setData(0, internalfmt, w, h, GL_RG_INTEGER, GL_INT, &testData[0]);
  testBufferOut.setStorage(1, internalfmt, w, h);

  testBuffer.bind();
  testBuffer.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  testBuffer.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  testBufferOut.bind();
  testBufferOut.parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  testBufferOut.parameter(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  //   testBufferOut.parameter(GL_TEXTURE_BASE_LEVEL, 0);
  //   testBufferOut.parameter(GL_TEXTURE_MAX_LEVEL, 1);



//  PrefixSumPlan plan(w,h, internalfmt, blockSize);
  SATPlan plan(w,h, internalfmt, blockSize);

  if (fullOutput)
    plan.enableDebugOutput();
  plan.execute(&testBuffer, &testBufferOut);

  bool success = true;

  if (cmpMem)
  {
    int bufSize = numTestVals * elemDim * 4;
    char* pActBuf = new char[bufSize];
#ifdef SAT_DBG
    ACG::GLDebug::getTextureData2D(testBufferOut.id(), 0, finfo.format(), finfo.type(), bufSize, pActBuf);
#endif

    if (memcmp(pActBuf, &expectedData[0], bufSize))
    {
      printf("error for %dx%d\n", w,h);
      success = false;
    }

    delete [] pActBuf;


    if (!success)
    {
      sprintf(szFileOut, SAT_DBG_DIR "2d_exp_%dx%d.bin", w,h);
      FILE* pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        fwrite(&expectedData[0], sizeof(expectedData[0]), expectedData.size(), pFile);
        fclose(pFile);
      }

#ifdef SAT_DBG
      sprintf(szFileOut, SAT_DBG_DIR "2d_act_%dx%d.bin", w,h);
      ACG::GLDebug::dumpTexture2D(testBufferOut.id(), 0, finfo.format(), finfo.type(), numTestVals * elemDim * 4, szFileOut, true);
#endif


      expectedData = testData;
      PrefixSumPlan::executeRowsCPU(w,h,expectedData);
      sprintf(szFileOut, SAT_DBG_DIR "2d_exp_rows_%dx%d.bin", w,h);
      pFile = fopen(szFileOut, "wb");
      if (pFile)
      {
        fwrite(&expectedData[0], sizeof(expectedData[0]), expectedData.size(), pFile);
        fclose(pFile);
      }
    }

  }

  return success;
}
