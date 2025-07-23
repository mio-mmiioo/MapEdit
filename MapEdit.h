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
	bool IsInMapEditArea() const { return isInMapEditArea_; } //マップエディタ領域内にいるかどうかを取得する

	void Update() override;
	void Draw() override;
	void SaveMapData();
	void LoadMapData();

private:
	void ClearMap();


	MapEditConfig cfg_;//マップチップの設定を保持する
	std::vector<int> myMap_; //マップの配列
	Rect mapEditRect_; //マップ領域の矩形
	Rect drawAreaRect_; //描画領域の矩形
	bool isInMapEditArea_; //マップエディタ領域内にいるかどうか

	std::vector<int> selectedChip_;

	Point ScrollOffset_; //スクロールオフセット これマップチップにも同じの書いてるから統一したい、、、
};

