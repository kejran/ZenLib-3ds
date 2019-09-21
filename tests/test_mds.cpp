#include <iostream>
#include <vdfs/fileIndex.h>
#include <gtest/gtest.h>

#include <zenload/modelScriptParser.h>
#include <zenload/zenParser.h>

TEST(MDS, ModelScriptBin) {
  ZenLoad::ZenParser    file("files/mds.bin");
  ZenLoad::MdsParserBin parser(file);

  while(true) {
    ZenLoad::MdsParser::Chunk c = parser.parse();
    if(c==ZenLoad::MdsParser::CHUNK_EOF)
      break;
    }
  }

TEST(MDS, ModelScriptTxt) {
  ZenLoad::ZenParser    file("files/mds.txt");
  ZenLoad::MdsParserTxt parser(file);

  while(true) {
    ZenLoad::MdsParser::Chunk c = parser.parse();
    if(c==ZenLoad::MdsParser::CHUNK_EOF)
      break;
    }
  }
