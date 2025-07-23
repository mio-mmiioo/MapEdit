#pragma once
#include "Library\GameObject.h"
#include <vector>
#include "globals.h"
#include "MapEditConfig.h"

namespace
{
	//const int MAP_WIDTH = { 20 };
	//const int MAP_HEIGHT = { 20 };
	//const int MAP_IMAGE_SIZE = { 32 }; // �摜�̃T�C�Y
	//const int LEFT_MARGIN = { 100 };
	//const int TOP_MARGIN = { 40 };
}



class MapEdit :
    public GameObject
{
public:
	MapEdit();
	~MapEdit();
	
	void SetMap(Point p, int value);
	int GetMap(Point p) const;
	bool IsInMapEditArea() const { return isInMapEditArea_; } //�}�b�v�G�f�B�^�̈���ɂ��邩�ǂ������擾����

	void Update() override;
	void Draw() override;
	void SaveMapData();
	void LoadMapData();
	void CopyMapData(Point p, Point mouse);

private:
	MapEditConfig cfg_;//�}�b�v�`�b�v�̐ݒ��ێ�����
	std::vector<int> myMap_; //�}�b�v�̔z��
	Rect mapEditRect_; //�}�b�v�̈�̋�`
	Rect drawAreaRect_; //�`��̈�̋�`
	bool isInMapEditArea_; //�}�b�v�G�f�B�^�̈���ɂ��邩�ǂ���

	std::vector<int> selectedChip_;
	std::vector<int> copyChips_;
};

