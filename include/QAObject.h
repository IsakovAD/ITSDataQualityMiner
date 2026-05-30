#pragma once 
#include <string>


struct QA_object{


	QA_object(): Name(""), Task(""), ObjectType("TH1") {};

	QA_object(string inName, string inTask, string inObjectType, bool in_isEnabled, bool in_isDoROF_norm, bool in_isLogy, bool in_isLogx):Name(inName),Task(inTask),ObjectType(inObjectType),isEnabled(in_isEnabled),isDoROF_norm(in_isDoROF_norm), isLogy(in_isLogy),isLogx(in_isLogx){};
	
	string Name;
	string Task;
	string ObjectType;
	bool isEnabled = false;
	bool isDoROF_norm= false;
	bool isLogy= false;
	bool isLogx=false;


};


