#include "PlayerData.h"

//Create blank PlayerData
PlayerData::PlayerData() {};

bool PlayerData::fromJSONString(PlayerData* p, std::string jsonString) {
	Json::Reader reader;
	bool res = reader.parse(jsonString.c_str(), p->jPlayerData);

	if (res) {
		// set ID
		p->setId(p->jPlayerData["id"].asInt()); 

		// set position
		float pos[3] = { p->jPlayerData["pos"][0].asFloat(),p->jPlayerData["pos"][1].asFloat(),p->jPlayerData["pos"][2].asFloat() };
		p->setPos(&pos[0]);

		// set region
		p->setRegion(p->jPlayerData["region"].asString());

		// set name
		p->setName(p->jPlayerData["name"].asString());
		return true;
			
	}
	else {
		//Invalid json, so parse the blank template
		reader.parse("{\"id\":0,\"pos\":[0,0,0],\"region\":\"Empty Space\",\"name\":\"Wanderer\"}", p->jPlayerData);
		return false;
	}
}

std::string PlayerData::getJSONString() {
	jPlayerData["id"] = _id;
	jPlayerData["pos"][0] = _pos[0];
	jPlayerData["pos"][1] = _pos[1]; 
	jPlayerData["pos"][2] = _pos[2];
	jPlayerData["region"] = _region;
	jPlayerData["name"] = _name;
	return jPlayerData.toStyledString();
}

int PlayerData::getId() {
	return _id;
}

void PlayerData::setId(int id) {
	_id = id;
}

float* PlayerData::getPos() {
	return _pos;
}

void PlayerData::setPos(float* pos) {
	memcpy(&_pos[0], pos, sizeof(_pos));
}

std::string PlayerData::getRegion() {
	return _region;
}

void PlayerData::setRegion(std::string region) {
	_region = region;
}

std::string PlayerData::getName() {
	return _name;
}

void PlayerData::setName(std::string name) {
	_name = name;
}


