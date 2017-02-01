/******************************************************************************
** Copyright (c) 2015, Intel Corporation                                     **
** All rights reserved.                                                      **
**                                                                           **
** Redistribution and use in source and binary forms, with or without        **
** modification, are permitted provided that the following conditions        **
** are met:                                                                  **
** 1. Redistributions of source code must retain the above copyright         **
**    notice, this list of conditions and the following disclaimer.          **
** 2. Redistributions in binary form must reproduce the above copyright      **
**    notice, this list of conditions and the following disclaimer in the    **
**    documentation and/or other materials provided with the distribution.   **
** 3. Neither the name of the copyright holder nor the names of its          **
**    contributors may be used to endorse or promote products derived        **
**    from this software without specific prior written permission.          **
**                                                                           **
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       **
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         **
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR     **
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT      **
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,    **
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED  **
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR    **
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    **
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      **
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        **
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
* ******************************************************************************/
/* Narayanan Sundaram (Intel Corp.)
 * ******************************************************************************/

#include <iostream>
#include <fstream>
#include <string>
#include "catch.hpp"
#include "generator.h"
#include "test_utils.h"

template<typename T>
void test_read_mtx(int n) {
  auto E = generate_dense_edgelist<T>(n);

  std::string tempfilenamestr = "GM_tempfileXXXXXX" + std::to_string(GraphMat::get_global_myrank());
  int suffixlen = std::to_string(GraphMat::get_global_myrank()).size();
  char* tempfilename = new char[tempfilenamestr.size()+1];
  std::cout << tempfilenamestr << " " << suffixlen << std::endl;

  int fd;
  do {
    memcpy(tempfilename, tempfilenamestr.c_str(), tempfilenamestr.size()*sizeof(char));
    tempfilename[tempfilenamestr.size()] = '\0';
    fd = mkstemps(tempfilename, suffixlen);
  } while(fd == -1);
  REQUIRE(fd != -1);
 
  char* tempfilenamewithoutsuffix = new char[tempfilenamestr.size() - suffixlen + 1];
  memcpy(tempfilenamewithoutsuffix, tempfilename, (tempfilenamestr.size() - suffixlen)*sizeof(char));
  tempfilenamewithoutsuffix[ tempfilenamestr.size() - suffixlen] = '\0';

  GraphMat::edgelist_t<T> E2;
  bool edgeweights;

  SECTION("Test text format with header and edgeweights") 
  {
  edgeweights = true;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, false, true, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, false, true, edgeweights);
  }

  SECTION("Test text format with no header and edgeweights") 
  {
  edgeweights = true;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, false, false, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, false, false, edgeweights);
  } 

  SECTION("Test text format with no header and no edgeweights") 
  {
  edgeweights = false;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, false, false, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, false, false, edgeweights);
  }

  SECTION("Test text format with header and no edgeweights") 
  {
  edgeweights = false;
  GraphMat::write_edgelist<T>(tempfilenamewithoutsuffix, E, false, true, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, false, true, edgeweights);
  }

  SECTION("Test binary format with header and edgeweights") 
  {
  edgeweights = true;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, true, true, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, true, true, edgeweights);
  }

  SECTION("Test binary format with no header and edgeweights") 
  {
  edgeweights = true;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, true, false, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, true, false, edgeweights);
  } 

  SECTION("Test binary format with no header and no edgeweights") 
  {
  edgeweights = false;
  GraphMat::write_edgelist(tempfilenamewithoutsuffix, E, true, false, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, true, false, edgeweights);
  }

  SECTION("Test binary format with header and no edgeweights") 
  {
  edgeweights = false;
  GraphMat::write_edgelist<T>(tempfilenamewithoutsuffix, E, true, true, edgeweights); //text format with header and edgeweights
  GraphMat::load_edgelist<T>(tempfilenamewithoutsuffix, &E2, true, true, edgeweights);
  } 

  unlink(tempfilename);

  GraphMat::edgelist_t<T> E_out;
  collect_edges(E, E_out);
  std::sort(E_out.edges, E_out.edges + E_out.nnz, edge_compare<T>);

  GraphMat::edgelist_t<T> E2_out;
  collect_edges(E2, E2_out);
  std::sort(E2_out.edges, E2_out.edges + E2_out.nnz, edge_compare<T>);

  REQUIRE(E_out.nnz == E2_out.nnz);
  for (int i = 0; i < E_out.nnz; i++) {
    REQUIRE(E_out.edges[i].src == E2_out.edges[i].src);
    REQUIRE(E_out.edges[i].dst == E2_out.edges[i].dst);
    if (edgeweights) REQUIRE(E_out.edges[i].val == E2_out.edges[i].val);
  }
  E.clear();
  E_out.clear();
  E2.clear();
  E2_out.clear();
}

TEST_CASE("IO") 
{
  SECTION("Test file IO (int)") {
    test_read_mtx<int>(10);
  }
  SECTION("Test file IO (float)") {
    test_read_mtx<float>(10);
  }
}

