// ======================================================================== //
// Copyright 2018-2020 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "umesh/io/IO.h"
#include "umesh/UMesh.h"

namespace umesh {
  namespace io {

    struct UGrid32Loader {
      
      typedef enum
        { /* try to detect automatically from file name: */
         AUTO,
         DOUBLE,
         FLOAT
        } VertexFormat;
      
      UGrid32Loader(const VertexFormat vertexFormat,
                    const std::string &dataFileName,
                    const std::string &scalarFileName);

      static UMesh::SP load(const VertexFormat vertexFormat,
                            const std::string &dataFileName,
                            const std::string &scalarFileName="");
      static UMesh::SP load(const std::string &dataFileName,
                            const std::string &scalarFileName="")
      { return load(AUTO,dataFileName,scalarFileName); }
      
      UMesh::SP result;
    };

  }
}
