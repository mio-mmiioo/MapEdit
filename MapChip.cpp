#define NOMINMAX

#include "MapChip.h"
#include "globals.h"
#include "Source\Screen.h"
#include "Input.h"
#include "ImGui/imgui.h"
#include <algorithm>


namespace
{
}

MapChip::MapChip()
	: GameObject(), isUpdate_(false), isInMapChipArea_(false), selectedIndex_(-1), 
	cfg_(GetMapChipConfig()), selected_({ 0,0 }), isHold_(false), ScrollOffset_({ 0,0 })
{
	bgHandle.resize(cfg_.TILE_PIX_SIZE * cfg_.TILES_X * cfg_.TILES_Y, -1);
	LoadDivGraph("./bg.png", cfg_.TILES_X * cfg_.TILES_Y, cfg_.TILES_X, cfg_.TILES_Y, cfg_.TILE_PIX_SIZE, cfg_.TILE_PIX_SIZE, bgHandle.data());

	for (int i = 0; i < bgHandle.size();i++)
	{
		HandleToIndex[bgHandle[i]] = i;
	}
}


MapChip::~MapChip()
{
	for (int i = 0;i < cfg_.TILES_X * cfg_.TILES_Y;i++) {
		if (bgHandle[i] != -1) {
			DeleteGraph(bgHandle[i]);
			bgHandle[i] = -1;
		}
	}
}

Point MapChip::GetViewOrigin() const {
	return { Screen::WIDTH - cfg_.MAPCHIP_WIN_WIDTH,0 };
}

bool MapChip::IsInChipArea(const Point& mouse) const
{
	return (mouse.x >= GetViewOrigin().x &&
		mouse.x < Screen::WIDTH &&
		mouse.y >= 0 &&
		mouse.y < cfg_.MAPCHIP_WIN_HEIGHT);
}

Point MapChip::ScreenToChipIndex(const Point& mouse) const
{
	int localX = (mouse.x - GetViewOrigin().x) / cfg_.TILE_PIX_SIZE;
	int localY = mouse.y / cfg_.TILE_PIX_SIZE;
	return { localX, localY };
}

void MapChip::Update()
{
	Point mousePos;
	if (GetMousePoint(&mousePos.x, &mousePos.y) == -1) return;

	isInMapChipArea_ = IsInChipArea(mousePos);

	if (isInMapChipArea_) {
		if (Input::IsKeyDown(KEY_INPUT_LEFT))  
			ScrollOffset_.x = std::max(0, ScrollOffset_.x - 1);
		if (Input::IsKeyDown(KEY_INPUT_RIGHT)) 
			ScrollOffset_.x = std::min(std::max(0, cfg_.TILES_X - cfg_.MAPCHIP_VIEW_X), ScrollOffset_.x + 1);
		if (Input::IsKeyDown(KEY_INPUT_UP))    
			ScrollOffset_.y = std::max(0, ScrollOffset_.y - 1);
		if (Input::IsKeyDown(KEY_INPUT_DOWN))  
			ScrollOffset_.y = std::min(std::max(0, cfg_.TILES_Y - cfg_.MAPCHIP_VIEW_Y), ScrollOffset_.y + 1);

		//マウスの座標をマップチップのインデックスに変換
		selected_ = ScreenToChipIndex(mousePos);

		int gx = selected_.x + ScrollOffset_.x;
		int gy = selected_.y + ScrollOffset_.y;
		int index = gy * cfg_.TILES_X + gx;

		//マウスの座標がマップチップの範囲内にあるかどうかをチェック
		if (index >= 0 && index < bgHandle.size() && Input::IsButtonDown(MOUSE_INPUT_LEFT)) {
			selectedIndex_ = bgHandle[index];
			isHold_ = true;
		}
	}
	else {
		isInMapChipArea_ = false;
	}
}

void MapChip::Draw()
{
	const int originX = GetViewOrigin().x;
	const int originY = GetViewOrigin().y;

	// チップ表示
	for (int y = 0; y < cfg_.MAPCHIP_VIEW_Y; y++) {
		for (int x = 0; x < cfg_.TILES_X; x++) {

			int gx = x + ScrollOffset_.x;
			int gy = y + ScrollOffset_.y;
			int index = gy * cfg_.TILES_X + gx;
			DrawGraph(originX + x * cfg_.TILE_PIX_SIZE,
				originY + y * cfg_.TILE_PIX_SIZE,
				bgHandle[index], TRUE);
		}
	}

	// ハイライト描画
	if (isInMapChipArea_) {
		int x = originX + selected_.x * cfg_.TILE_PIX_SIZE;
		int y = selected_.y * cfg_.TILE_PIX_SIZE;
		int size = cfg_.TILE_PIX_SIZE;

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
		DrawBox(x + 1, y - 1, x + size - 1, y + size + 1, GetColor(255, 255, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawBox(x, y, x + size, y + size, GetColor(255, 0, 0), FALSE);
	}

	// ホールド中チップの描画
	if (isHold_) {
		Point mousePos;
		if (GetMousePoint(&mousePos.x, &mousePos.y) != -1) {
			DrawExtendGraph(mousePos.x, mousePos.y,
				mousePos.x + cfg_.TILE_PIX_SIZE,
				mousePos.y + cfg_.TILE_PIX_SIZE,
				selectedIndex_, TRUE);
		}
		if (Input::IsButtonUP(MOUSE_INPUT_RIGHT)) {
			isHold_ = false;
			selectedIndex_ = -1;
		}
	}
}

bool MapChip::IsHold()
{
	return isHold_;
}

int MapChip::GetHoldImage()
{
	if (isHold_)
	{
		return selectedIndex_;
	}
	else
	{
		return -1; //持っていない場合は-1を返す
	}
}

int MapChip::GetChipIndex(int handle)
{
	return HandleToIndex[handle];
}
