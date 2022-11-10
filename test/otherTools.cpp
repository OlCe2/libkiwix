/*
 * Copyright (C) 2022 Veloman Yunkan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#include "gtest/gtest.h"
#include "../src/tools/otherTools.h"
#include "zim/suggestion_iterator.h"

#include <regex>

namespace
{

// Output generated via mustache templates sometimes contains end-of-line
// whitespace. This complicates representing the expected output of a unit-test
// as C++ raw strings in editors that are configured to delete EOL whitespace.
// A workaround is to put special markers (//EOLWHITESPACEMARKER) at the end
// of such lines in the expected output string and remove them at runtime.
// This is exactly what this function is for.
std::string removeEOLWhitespaceMarkers(const std::string& s)
{
  const std::regex pattern("//EOLWHITESPACEMARKER");
  return std::regex_replace(s, pattern, "");
}

} // unnamed namespace

#define CHECK_SUGGESTIONS(actual, expected) \
        EXPECT_EQ(actual, removeEOLWhitespaceMarkers(expected))

TEST(Suggestions, basicTest)
{
  kiwix::Suggestions s;
  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  //EOLWHITESPACEMARKER
]
)EXPECTEDJSON"
  );

  s.add(zim::SuggestionItem("Title", "/PATH", "Snippet"));

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title",
    "label" : "Snippet",
    "kind" : "path"
      , "path" : "/PATH"
  }
]
)EXPECTEDJSON"
  );

  s.add(zim::SuggestionItem("Title Without Snippet", "/P/a/t/h"));
  s.addFTSearchSuggestion("en", "kiwi");

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title",
    "label" : "Snippet",
    "kind" : "path"
      , "path" : "/PATH"
  },
  {
    "value" : "Title Without Snippet",
    "label" : "Title Without Snippet",
    "kind" : "path"
      , "path" : "/P/a/t/h"
  },
  {
    "value" : "kiwi ",
    "label" : "containing &apos;kiwi&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
  );
}

TEST(Suggestions, specialCharHandling)
{
  // HTML special symbols (<, >, &, ", and ') must be HTML-escaped
  // Backslash symbols (\) must be duplicated.
  const std::string SPECIAL_CHARS(R"(\<>&'")");
  {
    kiwix::Suggestions s;
    s.add(zim::SuggestionItem("Title with "   + SPECIAL_CHARS,
                              "Path with "    + SPECIAL_CHARS,
                              "Snippet with " + SPECIAL_CHARS));

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Title with \\&lt;&gt;&amp;&apos;&quot;",
    "label" : "Snippet with \\&lt;&gt;&amp;&apos;&quot;",
    "kind" : "path"
      , "path" : "Path with \\&lt;&gt;&amp;&apos;&quot;"
  }
]
)EXPECTEDJSON"
    );
  }

  {
    kiwix::Suggestions s;
    s.add(zim::SuggestionItem("Snippetless title with " + SPECIAL_CHARS,
                              "Path with " + SPECIAL_CHARS));

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "Snippetless title with \\&lt;&gt;&amp;&apos;&quot;",
    "label" : "Snippetless title with \\&lt;&gt;&amp;&apos;&quot;",
    "kind" : "path"
      , "path" : "Path with \\&lt;&gt;&amp;&apos;&quot;"
  }
]
)EXPECTEDJSON"
    );
  }

  {
    kiwix::Suggestions s;
    s.addFTSearchSuggestion("eng", "text with " + SPECIAL_CHARS);

    CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "text with \\&lt;&gt;&amp;&apos;&quot; ",
    "label" : "containing &apos;text with \\&lt;&gt;&amp;&apos;&quot;&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
    );
  }
}

TEST(Suggestions, fulltextSearchSuggestionIsTranslated)
{
  kiwix::Suggestions s;
  s.addFTSearchSuggestion("it", "kiwi");

  CHECK_SUGGESTIONS(s.getJSON(),
R"EXPECTEDJSON([
  {
    "value" : "kiwi ",
    "label" : "contenente &apos;kiwi&apos;...",
    "kind" : "pattern"
    //EOLWHITESPACEMARKER
  }
]
)EXPECTEDJSON"
  );
}
