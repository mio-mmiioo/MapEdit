#include "Stage.h"
#include "Input.h"

namespace
{

}

Stage::Stage()
	: GameObject()
{
	mapEdit_ = new MapEdit(); //マップエディタのインスタンスを作成
	mapChip_ = new MapChip(); //マップチップのインスタンスを作成
}

Stage::~Stage()
{
	//for (int i = 0;i < MAP_CHIP_WIDTH * MAP_CHIP_HEIGHT;i++) {
	//	if (bgHandle_[i] != -1) {
	//		DeleteGraph(bgHandle_[i]);
	//		bgHandle_[i] = -1;
	//	}
	//}
	//delete[] bgHandle; //配列の解放 自分で撮ったものは自分で消す
	delete mapChip_; //マップチップのインスタンスを解放
	//delete mapEdit_; //マップエディタのインスタンスを解放
}

void Stage::Update()
{
}

void Stage::Draw()
{

}


