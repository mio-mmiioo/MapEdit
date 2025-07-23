#pragma once
#include "Library\GameObject.h"
#include <vector>
#include "globals.h"
#include "MapEditConfig.h"

namespace
{
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

private:
	void ClearMap();


	MapEditConfig cfg_;//�}�b�v�`�b�v�̐ݒ��ێ�����
	std::vector<int> myMap_; //�}�b�v�̔z��
	Rect mapEditRect_; //�}�b�v�̈�̋�`
	Rect drawAreaRect_; //�`��̈�̋�`
	bool isInMapEditArea_; //�}�b�v�G�f�B�^�̈���ɂ��邩�ǂ���

	std::vector<int> selectedChip_;

	Point ScrollOffset_; //�X�N���[���I�t�Z�b�g ����}�b�v�`�b�v�ɂ������̏����Ă邩�瓝�ꂵ�����A�A�A
};

