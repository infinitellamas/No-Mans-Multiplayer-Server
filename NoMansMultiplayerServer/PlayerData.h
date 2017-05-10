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

		std::chrono::milliseconds lastResponseTime = (std::chrono::milliseconds)0;

private:
	Json::Value jPlayerData; // We use this to easily serialize/de-serialize
	int _id = 0;
	float _pos[3] = { 0,0,0 };
	std::string _region = "Empty Space"; // Default region.
	std::string _name = "Wanderer"; // Default name.
};