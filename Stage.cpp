#include "Stage.h"
#include "Input.h"

namespace
{

}

Stage::Stage()
	: GameObject()
{
	mapEdit_ = new MapEdit(); //�}�b�v�G�f�B�^�̃C���X�^���X���쐬
	mapChip_ = new MapChip(); //�}�b�v�`�b�v�̃C���X�^���X���쐬
}

Stage::~Stage()
{
	//for (int i = 0;i < MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT;i++) {
	//	if (bgHandle_[i] != -1) {
	//		DeleteGraph(bgHandle_[i]);
	//		bgHandle_[i] = -1;
	//	}
	//}
	//delete[] bgHandle; //�z��̉�� �����ŎB�������͎̂����ŏ���
	delete mapChip_; //�}�b�v�`�b�v�̃C���X�^���X�����
	//delete mapEdit_; //�}�b�v�G�f�B�^�̃C���X�^���X�����
}

void Stage::Update()
{
}

void Stage::Draw()
{

}


