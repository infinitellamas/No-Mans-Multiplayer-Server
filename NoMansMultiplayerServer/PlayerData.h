#pragma once
//This header is included in the client and server and defines datatypes and conversions to aid the client-server transmission of data
#include <string>
#include <sstream>
#include <json/json.h>
#include <iostream>
#include <chrono>

class PlayerData {
public:
		PlayerData();
		static bool fromJSONString(PlayerData* p,std::string jsonString);
		std::string getJSONString();
		int getId();
		void setId(int id);
		float* getPos();
		void setPos(float* pos);
		std::string getRegion();
		void setRegion(std::string region);
		std::string getName();
		void setName(std::string name);

private:
	Json::Value jPlayerData; // We use this to easily serialize/de-serialize
	int _id = 0;
	float _pos[3] = { 0,0,0 };
	char _region[30] = "Empty Space"; // Default region.
	char _name[30] = "Wanderer"; // Default name.
};