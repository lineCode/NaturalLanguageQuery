#include "StdAfx.h"
#include "NLPHelper.h"
#include "CMDLexer.hpp"
#include "CMDParser.hpp"
#include "MDSearch.h"
#include "TextMining/TextAnalysis/TextAnalysis.h"

using namespace User;
using namespace std;

bool string2Guid(GUID& orGUID, const std::string &s)
{
    wstring ws(s.begin(), s.end());
	if (MBase::String2Guid(orGUID, ws.c_str()))
		return true;
	else
		return false;
}

int testAntlrCMD(const string iFileName, const string oFileName)
{
	MDSearch instance;
	instance.Init("./TAConf/NLPHelper/NLPHelper.conf");
	vector<SearchResult> results;

	ifstream file;
	file.open(iFileName.c_str(), ios::binary);
	if (!file) {
		cerr << "Error: read json file failed!" << endl;
		return -1;
	}

	Json::Reader reader;
	Json::Value root;

	if (reader.parse(file, root)) {
		ofstream output(oFileName.c_str(),std::ofstream::binary);
		//CDSSNLPParser iNLPParser = new CDSSNLPParser();
		for (unsigned i = 0; i < root.size(); i++) {
			vector<TAChunkTerm> vChunkTerms;
			for (unsigned j = 0; j < root[i]["terms"].size(); j++) {
				vChunkTerms.push_back(TAChunkTerm(root[i]["terms"][j][0].asInt(), root[i]["terms"][j][1].asString(), root[i]["terms"][j][2].asString(),
					root[i]["terms"][j][3].asString(), root[i]["terms"][j][4].asString(), root[i]["terms"][j][5].asString()));
			}

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
			
			output << "Test case " << i << ": " << root[i]["query"].asString() << endl;
//			psr.cmd(vChunkTerms, &instance, false, &output);
			output << endl;
		}
		output.close();
	}

	file.close();

	return 0;
}

void writeChunkTerm(const string iFileName, const string oFileName)
{
	TextAnalysis instance;
	instance.Init("./TAConf/TextAnalysis/lexalytics.properties.default");
	instance.CreateSession();

	vector<string> vQuery;
	ifstream iFile(iFileName.c_str());
	if (iFile.is_open()) {
		string line;
		while (getline(iFile, line))
			vQuery.push_back(line);
		iFile.close();
	}

	Json::Value root;

	for (int i = 0; i < vQuery.size(); i++) {
		vector<TAChunkTerm> chunkTerms;
		instance.Process(vQuery.at(i), chunkTerms);

		Json::Value element;
		element["query"] = vQuery.at(i);

		Json::Value jsonArray;
		for (int j = 0; j < chunkTerms.size(); j++) {
			jsonArray[j].append(chunkTerms[j].mChunkId);
			jsonArray[j].append(chunkTerms[j].mLabel);
			jsonArray[j].append(chunkTerms[j].mTermTxt);
			jsonArray[j].append(chunkTerms[j].mStemTermTxt);
			jsonArray[j].append(chunkTerms[j].mPosTag);
			jsonArray[j].append(chunkTerms[j].mEntityType);
		}

		element["terms"] = jsonArray;
		root.append(element);
	}

	ofstream file(oFileName.c_str());
	if (file.is_open()) {
		file << root.toStyledString() << endl;
		file.close();
	} else {
		cerr << "Error: open json file failed" << endl;
	}
}

void compareOutput(const string iCompareFile, const string iOutputFile)
{
	ifstream compareFile(iCompareFile.c_str());
	ifstream outputFile(iOutputFile.c_str());
	if (compareFile.is_open() && outputFile.is_open()) {
		string compareLine;
		string outputLine;
		vector<string> vCompare;
		vector<string> vOutput;
		int num = 0;
		bool isResultSame = true;
		while (getline(compareFile, compareLine)) {
			string::size_type position = compareLine.find(":"); 
			string query;
			if (position != string::npos)
				query = compareLine.substr(position + 2);
			vCompare.clear();
			vOutput.clear();
			if (compareLine.substr(0, 4) == "Test") {
				while (getline(compareFile, compareLine)) {
					if (compareLine != "") 
						vCompare.push_back(compareLine);
					else
						break;
				}
			}
			getline(outputFile, outputLine);
			if (outputLine.substr(0, 4) == "Test") {
				while (getline(outputFile, outputLine)) {
					if (outputLine != "")
						vOutput.push_back(outputLine);
					else
						break;
				}
			}
			bool flag = false;
			if (vCompare.size() == vOutput.size()) {
				unsigned i;
				for (i = 0; i < vCompare.size(); i++) {
					if (vCompare[i] != vOutput[i]) {
						isResultSame = false;
						cout << endl << "Result of test case " << num << " is different." << endl;
						break;
					}
				}
				if (i == vCompare.size()) {
					flag = true;
				}
			} else {
				isResultSame = false;
				cout << endl << "Result of test case " << num << " is different." << endl;
			}
			if (!flag) {
				cout << query << endl;
				cout << "The previous result:" << endl;
				for (unsigned i = 0; i < vCompare.size(); i++)
					cout << "\t" << vCompare[i] << endl;
				cout << "The output result:" << endl;
				for (unsigned i = 0; i < vOutput.size(); i++)
					cout << "\t" << vOutput[i] << endl;
				cout << endl;
			}
			num++;
		}
		if (isResultSame) 
			cout << endl << "Congratulations! Result of all the test cases are the same!" << endl << endl;
		else
			cout << "Result of the other test cases are the same." << endl << endl;
		if (getline(outputFile, outputLine)) {
			cout << "Output file has more query test cases than previous file." << endl;
			cout << "Update the stored output file for comparing." << endl << endl;
		}
	}
}