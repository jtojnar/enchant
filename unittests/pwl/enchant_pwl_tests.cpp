/* Copyright (c) 2007 Eric Scott Albright
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#define NOMINMAX //don't want windows to collide with std::min
#include <UnitTest++.h>
#include <enchant.h>
#include <enchant-provider.h>

#include "../EnchantDictionaryTestFixture.h"

struct EnchantPwl_TestFixture : EnchantDictionaryTestFixture
{
    //Setup
    EnchantPwl_TestFixture(const std::string& languageTag="qaa"):
        EnchantDictionaryTestFixture(EmptyDictionary_ProviderConfiguration, languageTag)
    { }
};

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryBeginsWithBOM
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryBeginsWithBOM_Successful)
{
	std::string Utf8Bom ("\xef\xbb\xbf");
	ExternalAddWordToDictionary(Utf8Bom + "cat");

	ReloadTestDictionary();

    CHECK( IsWordInDictionary("cat") );
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// DictionaryHasInvalidUtf8
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasInvalidUtf8Data_OnlyReadsValidLines)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<const std::string>::const_iterator bad = sWords.insert(sWords.begin()+2, "\xa5\xf1\x08"); //invalid utf8 data
  ExternalAddWordsToDictionary(sWords);

  ReloadTestDictionary();

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin(); itWord != bad; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*bad) );

  for(std::vector<const std::string>::const_iterator itWord = bad+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////
// Pwl Bugs
TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_DistanceUsesUnicodeChar)
{
	std::string puaWord("\xF4\x80\x80\x80ord"); // private use character 
	AddWordToDictionary(puaWord); //edit distance 1 using unichar; 4 using utf8

    std::vector<const std::string> suggestions = GetSuggestionsFromWord("word");

	CHECK( !suggestions.empty());

	if(!suggestions.empty()){
		CHECK_EQUAL(puaWord, suggestions[0]);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Unicode normalization
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasComposed_SuccessfulCheckWithComposedAndDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  ReloadTestDictionary();

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD u0301 = Combining acute accent
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedComposed_SuccessfulCheckWithComposedAndDecomposed)
{
  AddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD u0301 = Combining acute accent
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_DictionaryHasDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  ReloadTestDictionary();

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedDecomposed_SuccessfulCheckWithComposedAndDecomposed)
{
  AddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  CHECK( IsWordInDictionary(Convert(L"fianc\xe9")) ); //NFC
  CHECK( IsWordInDictionary(Convert(L"fiance\x301")) ); //NFD
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_DictionaryHasComposed_ReturnsComposed)
{
  ExternalAddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  ReloadTestDictionary();

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_AddedComposed_ReturnsComposed)
{
  AddWordToDictionary(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"fianc\xe9")); // u00e9 = Latin small letter e with acute
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_DictionaryHasDecomposed_ReturnsDecomposed)
{
  ExternalAddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  ReloadTestDictionary();

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"fiance\x301"));  // u0301 = Combining acute accent
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             Suggest_AddedDecomposed_ReturnsDecomposed)
{
  AddWordToDictionary(Convert(L"fiance\x301")); // u0301 = Combining acute accent

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("fiance");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"fiance\x301")); // u0301 = Combining acute accent
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Capitalization
TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedAllCaps_OnlyAllCapsSuccessful)
{
  AddWordToDictionary("CIA");

  CHECK( IsWordInDictionary("CIA") );
  CHECK(!IsWordInDictionary("CIa") );
  CHECK(!IsWordInDictionary("Cia") );
  CHECK(!IsWordInDictionary("cia") );
  CHECK(!IsWordInDictionary("cIa") );

  CHECK( IsWordInSession("CIA") );
  CHECK(!IsWordInSession("CIa") );
  CHECK(!IsWordInSession("Cia") );
  CHECK(!IsWordInSession("cia") );
  CHECK(!IsWordInSession("cIa") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedTitle_lowerCaseAndMixedCaseNotSuccessful)
{
  AddWordToDictionary("Eric");

  CHECK( IsWordInDictionary("ERIC") );
  CHECK(!IsWordInDictionary("ERic") );
  CHECK( IsWordInDictionary("Eric") );
  CHECK(!IsWordInDictionary("eric") );
  CHECK(!IsWordInDictionary("eRic") );

  CHECK( IsWordInSession("ERIC") );
  CHECK(!IsWordInSession("ERic") );
  CHECK( IsWordInSession("Eric") );
  CHECK(!IsWordInSession("eric") );
  CHECK(!IsWordInSession("eRic") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_Addedlower_MixedCaseNotSuccessful)
{
  AddWordToDictionary("rice");

  CHECK( IsWordInDictionary("RICE") );
  CHECK(!IsWordInDictionary("RIce") );
  CHECK( IsWordInDictionary("Rice") );
  CHECK( IsWordInDictionary("rice") );
  CHECK(!IsWordInDictionary("rIce") );

  CHECK( IsWordInSession("RICE") );
  CHECK(!IsWordInSession("RIce") );
  CHECK( IsWordInSession("Rice") );
  CHECK( IsWordInSession("rice") );
  CHECK(!IsWordInSession("rIce") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedAllCapsOfTrueTitleCase_OnlyAllCapsSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedTrueTitleCase_lowerCaseAndMixedCaseNotSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK(!IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             IsWordInDictionary_AddedLowerOfTrueTitleCase_MixedCaseNotSuccessful)
{
  AddWordToDictionary(Convert(L"\x01f3ie")); // u01f2 is Latin small letter dz

  CHECK( IsWordInDictionary(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInDictionary(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInDictionary(Convert(L"\x01f3Ie")) );

  CHECK( IsWordInSession(Convert(L"\x01f1IE")) ); // u01f1 is Latin captial letter Dz
  CHECK(!IsWordInSession(Convert(L"\x01f2IE")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f2ie")) ); // u01f2 is Latin capital letter d with small letter z
  CHECK( IsWordInSession(Convert(L"\x01f3ie")) ); // u01f3 is Latin small letter dz
  CHECK(!IsWordInSession(Convert(L"\x01f3Ie")) );
}

///////////////////////////////////////////////////////////////////////////////////////////
// Capitalization on Suggestions
TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("CEA");

  std::vector<const std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordTitleCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Cea");

  std::vector<const std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCaps_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIA");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("cea");

  std::vector<const std::string> expected;
  expected.push_back("CIA");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("Eric");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("RIC");

  std::vector<const std::string> expected;
  expected.push_back("ERIC");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary("Eric");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Ric");

  std::vector<const std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCase_WordLowerCase_SuggestionTitleCase)
{
  AddWordToDictionary("Eric");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("ric");

  std::vector<const std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary("rice");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("RIC");

  std::vector<const std::string> expected;
  expected.push_back("RICE");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary("rice");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Ric");

  std::vector<const std::string> expected;
  expected.push_back("Rice");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCase_WordLowerCase_SuggestionLowerCase)
{
  AddWordToDictionary("rice");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("ric");

  std::vector<const std::string> expected;
  expected.push_back("rice");
  CHECK_EQUAL(expected.size(), suggestions.size());
  CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
}






TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordTitleCase_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsOfTrueTitleCase_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f1IE"));  // u01f1 is Latin captial letter Dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseOfTrueTitleCase_WordLowerCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f2ie"));  // u01f2 is Latin capital letter d with small letter z

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordAllCaps_SuggestionAllCaps)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("RIE");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f1IE")); // u01f1 is Latin captial letter Dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordTitleCase_SuggestionTitleCase)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("Rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f2ie")); // u01f2 is Latin capital letter d with small letter z
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedLowerCaseOfTrueTitleCase_WordLowerCase_SuggestionLowerCase)
{
  AddWordToDictionary(Convert(L"\x01f3ie"));  // u01f3 is Latin small letter dz

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("rie");

  std::vector<const std::string> expected;
  expected.push_back(Convert(L"\x01f3ie")); // u01f3 is Latin small letter dz
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}


TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedAllCapsWithPrefix_WordLowerCase_SuggestionAllCaps)
{
  AddWordToDictionary("CIAL");
  AddWordToDictionary("CIALAND");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("ceal");

  std::vector<const std::string> expected;
  expected.push_back("CIAL");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             GetSuggestions_AddedTitleCaseWithPrefix_WordLowerCase_SuggestionTitlecase)
{
  AddWordToDictionary("Eric");
  AddWordToDictionary("Ericson");

  std::vector<const std::string> suggestions = GetSuggestionsFromWord("eric");

  std::vector<const std::string> expected;
  expected.push_back("Eric");
  CHECK_EQUAL(expected.size(), suggestions.size());
  if(expected.size() == suggestions.size())
  {
      CHECK_ARRAY_EQUAL(expected, suggestions, expected.size());
  }
}

/////////////////////////////////////////////////////////////////////////////
// Remove from PWL
TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix1)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("help");

  CHECK(!IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix2)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix3)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");
  AddWordToDictionary("helm");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("help") );
  CHECK(!IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SharedPrefix4)
{
  AddWordToDictionary("help");
  AddWordToDictionary("hello");
  AddWordToDictionary("helm");

  CHECK( IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );

  RemoveWordFromDictionary("help");

  CHECK(!IsWordInDictionary("help") );
  CHECK( IsWordInDictionary("hello") );
  CHECK( IsWordInDictionary("helm") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_SingleWord)
{
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_MultipleWords1)
{
  AddWordToDictionary("special");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK(!IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_MultipleWords2)
{
  AddWordToDictionary("special");
  AddWordToDictionary("hello");

  CHECK( IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );

  RemoveWordFromDictionary("special");

  CHECK(!IsWordInDictionary("special") );
  CHECK( IsWordInDictionary("hello") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix1)
{
  AddWordToDictionary("ant");
  AddWordToDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("ant");

  CHECK(!IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix2)
{
  AddWordToDictionary("anteater");
  AddWordToDictionary("ant");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("ant");

  CHECK(!IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix3)
{
  AddWordToDictionary("ant");
  AddWordToDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK(!IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ProperPrefix4)
{
  AddWordToDictionary("anteater");
  AddWordToDictionary("ant");

  CHECK( IsWordInDictionary("ant") );
  CHECK( IsWordInDictionary("anteater") );

  RemoveWordFromDictionary("anteater");

  CHECK( IsWordInDictionary("ant") );
  CHECK(!IsWordInDictionary("anteater") );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromFile)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<const std::string>::const_iterator removed = sWords.insert(sWords.begin()+2, "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );

  for(std::vector<const std::string>::const_iterator itWord = removed+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromBeginningOfFile)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<const std::string>::const_iterator removed = sWords.insert(sWords.begin(), "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  CHECK(!IsWordInDictionary(*removed) );

  for(std::vector<const std::string>::const_iterator itWord = removed+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromBeginningOfFileWithBOM)
{
  std::string Utf8Bom ("\xef\xbb\xbf");

  std::vector<const std::string> sWords;
  sWords.push_back(Utf8Bom + "hello");
  sWords.push_back("cat");
  sWords.push_back("hat");

  ExternalAddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  CHECK(!IsWordInDictionary("hello") );

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin()+1; itWord != sWords.end(); ++itWord){
    CHECK(IsWordInDictionary(*itWord) );
  }
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_ItemRemovedFromEndOfFile)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");

  std::vector<const std::string>::const_iterator removed = sWords.insert(sWords.end(), "hello");
  AddWordsToDictionary(sWords);

  RemoveWordFromDictionary("hello");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );
}

TEST_FIXTURE(EnchantPwl_TestFixture, 
             PwlRemove_FileHasProperSubset_ItemRemovedFromFile)
{
  std::vector<const std::string> sWords;
  sWords.push_back("cat");
  sWords.push_back("hat");
  sWords.push_back("that");
  sWords.push_back("bat");
  sWords.push_back("tot");
  sWords.push_back("anteater");

  std::vector<const std::string>::const_iterator removed = sWords.insert(sWords.end(), "ant");
  AddWordsToDictionary(sWords);
  RemoveWordFromDictionary("ant");

  ReloadTestDictionary(); // to see what actually persisted

  for(std::vector<const std::string>::const_iterator itWord = sWords.begin(); itWord != removed; ++itWord){
    CHECK( IsWordInDictionary(*itWord) );
  }
  CHECK(!IsWordInDictionary(*removed) );
}
