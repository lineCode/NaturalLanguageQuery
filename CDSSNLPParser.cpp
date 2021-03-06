#include "StdAfx.h"
#include <sstream>
#include "CDSSNLPParser.h"
#include "DSSNode/CDSSNode.h"
#include "MDSearch.h"
#include "CMDLexer.hpp"
#include "CMDParser.hpp"
//#include "Base/Defines/RefContainer.h"

using namespace User;

TextAnalysis* CDSSNLPParser::TAInstance;

// entry point, report manipulation passes in user input
STDMETHODIMP CDSSNLPParser::Parse(BSTR iText)
{	
	if (!mpDataTemplate || !mpDataFilter)
		return E_UNEXPECTED;

	//HRESULT hr = hClear();
	//if (FAILED(hr))
	//	return hr;

	wstring iWString(iText);
	string iString = MBase::WideCharToUTF8(iWString.c_str()).c_str();

	MDSearch searchInstance;
	searchInstance.Init("./TAConf/NLPHelper/");

	//TextAnalysis TAInstance;
	//TAInstance.Init("./TAConf/TextAnalysis/lexalytics.properties.default");
	//TAInstance.CreateSession();

	vector<TAChunkTerm> vChunkTerms;
	TAInstance->Process(iString, vChunkTerms);
	//TAInstance.CloseSession();
	ofstream output("nlp_output.txt", std::ofstream::binary);
		
	string inputString;
	stringstream isString;
	for (int j = 0; j < vChunkTerms.size()-1; j++)
		isString << setfill('0') << setw(2) << j << ' ';
	isString << setfill('0') << setw(2) << vChunkTerms.size()-1;
	inputString = isString.str();
	CMDLexer::InputStreamType input((ANTLR_UINT8 *) inputString.c_str(), ANTLR_ENC_8BIT, (ANTLR_UINT32) inputString.length(), (ANTLR_UINT8 *) "test");
	//TLexer::InputStreamType input(fName, ANTLR_ENC_8BIT);
	CMDLexer lxr(&input);	    // CLexerNew is generated by ANTLR
	CMDParser::TokenStreamType tstream(ANTLR_SIZE_HINT, lxr.get_tokSource() );
	CMDParser psr(&tstream);  // CParserNew is generated by ANTLR3
	try {
		psr.cmd(vChunkTerms, &searchInstance, true, true, &output, this);
	} catch (exception &e) {
		cout << e.what() << endl;
	}

	HRESULT hr = hCleanup();
	if FAILED(hr) return hr;

    return S_OK;
}
