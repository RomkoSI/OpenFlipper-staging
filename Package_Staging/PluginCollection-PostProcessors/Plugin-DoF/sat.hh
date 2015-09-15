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

#pragma once

#include <ACG/GL/globjects.hh>

#include <QStringList>
#include <map>

class PrefixSumPlan
{
public:

  /*
  width of input texture must be a multiple of _blocksize!!
  if that's not the case, prepare the input before calling execute().
  width() will return the expected width, and padInput can do the padding if necessary
  */
  PrefixSumPlan(int _w, int _h, GLenum _internalFmt = GL_R32F, int _blocksize = 32);
  virtual ~PrefixSumPlan();



  // copy _src to _dst and pad _dst to expected dimension for execute()
  //  this will also initialize _dst with the correct width as necessary
  //  _padWidthAndHeight also pads the height to a multiple of blocksize (good for 2D SAT)
  bool padInput(ACG::Texture2D* _src, ACG::Texture2D* _dst, bool _padWidthAndHeight);


  bool execute(ACG::TextureBuffer* _src, ACG::TextureBuffer* _dst);
  bool execute(ACG::Texture2D* _src, ACG::Texture2D* _dst);


  // get padded! width
  int width() const {return width_;}
  // get height
  int height() const {return height_;}

  int blocksize() const {return blocksize_;}

  // size in bytes of one texel
  int elemSize() const {return elemSize_;}

  int paddedDimension(int _dim) const;
  int paddedBlocksize(int _size) const;

  const QStringList& macros() const {return macros_;}


  // debugging stuff
  void enableDebugOutput() {dbgOutput_ = 1;}
  void disableDebugOutput() {dbgOutput_ = 0;}
  bool debugOutputEnabled() {return dbgOutput_ != 0;}
  void debugSetTransposedInput(int i) {dbgTranposedInput_ = i;}

  void enableProfiling() {dbgProfile_ = 1;}
  void disableProfiling() {dbgProfile_ = 0;}
  bool profilingEnabled() const {return dbgProfile_ != 0;}


  // compute per row prefixsums on cpu, in-place, data in row-major alignment
  template<class T>
  static void executeRowsCPU(int _w, int _h, std::vector<T>& _inout);

  // compute per column prefixsums on cpu, in-place, data in row-major alignment
  template<class T>
  static void executeColsCPU(int _w, int _h, std::vector<T>& _inout);


  static bool testBuffer(int w, int cmpMem = 1, int fullOutput = 0);
  static bool test2D(int w, int h, int cmpMem = 1, int fullOutput = 0);

private:

  int width_, height_;
  int blocksize_;
  int numWorkGroupsX_;
  int numWorkGroupsY_;

  int numBlockScanGroupsX_;
  int numBlockScanGroupsY_;
  int numDispatches_;

  GLenum internalFmt_;
  // size in bytes of one texel
  int elemSize_;

  QStringList macros_;

  ACG::TextureBuffer blockSums_;
  ACG::TextureBuffer blockSumsOut_;
  ACG::Texture2D blockSums2D_;
  ACG::Texture2D blockSums2DOut_;

  PrefixSumPlan* blockSumPlan_;

  // debugging stuff
  int dbgOutput_;
  int dbgTranposedInput_;
  int dbgProfile_;


  ACG::QueryCounter perfCounter_;

  static std::map<GLenum, const char*> datatypeMacros_;
};




class SATPlan
{
public:

  SATPlan(int _w, int _h, GLenum _internalFmt = GL_R32F, int _blocksize = 32);
  virtual ~SATPlan();


  bool execute(ACG::Texture2D* _src, ACG::Texture2D* _dst);


  PrefixSumPlan* getRowPlan() const {return rows_;}
  PrefixSumPlan* getColPlan() const {return cols_;}


  void enableDebugOutput();
  void enableProfiling();

  template<class T>
  static void executeCPU(int _w, int _h, std::vector<T>& _inout);

private:

  bool transpose(ACG::Texture2D* _src, ACG::Texture2D* _dst);


  PrefixSumPlan* rows_;
  PrefixSumPlan* cols_;

  bool paddingRequired_;

  ACG::Texture2D paddedInput_;
  ACG::Texture2D transposedSrc_;
  ACG::Texture2D transposedDst_;
  QStringList transposeMacros_;
  int transposeGroupSize_;

  ACG::QueryCounter perfCounter_;
};





template<class T>
void PrefixSumPlan::executeRowsCPU( int w, int h, std::vector<T>& _inout )
{
  if (!_inout.empty())
  {
    for (int r = 0; r < h; ++r)
    {
      int offsetRow = r*w;

      for (int c = 1; c < w; ++c)
      {
        int offsetRd = offsetRow + c-1;
        int offsetWr = offsetRow + c;

        T x = _inout[offsetRow];
        _inout[offsetRow] = _inout[offsetWr];

        if (offsetRow == offsetRd)
          _inout[offsetWr] = x;
        else
          _inout[offsetWr] = _inout[offsetRd] + x;
      }

      // reset first row element to 0
      memset(&_inout[offsetRow], 0, sizeof(T));
    }
  }
}

template<class T>
void PrefixSumPlan::executeColsCPU( int w, int h, std::vector<T>& _inout )
{
  if (!_inout.empty())
  {
    for (int c = 0; c < w; ++c)
    {
      for (int r = 1; r < h; ++r)
      {
        int offsetRd = (r-1)*w + c;
        int offsetWr = r*w + c;

        T x = _inout[c];
        _inout[c] = _inout[offsetWr];

        if (c == offsetRd)
          _inout[offsetWr] = x;
        else
          _inout[offsetWr] = _inout[offsetRd] + x;
      }
    }
    
    // reset first column element to 0
    memset(&_inout[0], 0, sizeof(T) * w);
  }
}



template<class T>
void SATPlan::executeCPU( int w, int h, std::vector<T>& _inout )
{
  PrefixSumPlan::executeRowsCPU(w,h, _inout);
  PrefixSumPlan::executeColsCPU(w,h, _inout);
}