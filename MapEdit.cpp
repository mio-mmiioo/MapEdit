#define NOMINMAX

#include <windows.h>
#include "MapEdit.h"
#include <cassert>
#include "Input.h"
#include "DxLib.h"
#include "MapChip.h"
#include <fstream>
#include <codecvt>
#include <sstream>
#include <string>
#include <iostream>
#include "MapEditConfig.h"
#include <algorithm>

MapEdit::MapEdit()
	:GameObject(), cfg_(GetMapEditConfig()), myMap_(cfg_.MAP_WIDTH* cfg_.MAP_HEIGHT, -1),
	selectedChip_(cfg_.MAP_WIDTH* cfg_.MAP_HEIGHT, -1),
	ScrollOffset_({0, 0}),
	isInMapEditArea_(false) //マップエディタ領域内にいるかどうか
{
	mapEditRect_ = { cfg_.LEFT_MARGIN, cfg_.TOP_MARGIN, cfg_.MAP_WIDTH * cfg_.MAP_IMAGE_SIZE, cfg_.MAP_HEIGHT * cfg_.MAP_IMAGE_SIZE };
}

MapEdit::~MapEdit()
{
}

void MapEdit::SetMap(Point p, int value)
{
	//マップの座標pにvalueをセットする
	//pが、配列の範囲外の時はassertにひっかかる
	assert(p.x >= 0 && p.x < cfg_.MAP_WIDTH);
	assert(p.y >= 0 && p.y < cfg_.MAP_HEIGHT);
	myMap_[p.y * cfg_.MAP_WIDTH + p.x] = value; //y行x列にvalueをセットする
}

int MapEdit::GetMap(Point p) const
{
	//マップの座標pの値を取得する
	//pが、配列の範囲外の時はassertにひっかかる
	assert(p.x >= 0 && p.x < cfg_.MAP_WIDTH);
	assert(p.y >= 0 && p.y < cfg_.MAP_HEIGHT);
	//assert(p.x >= 0 && p.x < cfg_.MAP_EDIT_VIEW_X + ScrollOffset_.x);
	//assert(p.y >= 0 && p.y < cfg_.MAP_EDIT_VIEW_Y + ScrollOffset_.y);
	return myMap_[p.y * cfg_.MAP_WIDTH + p.x]; //y行x列の値を取得する
}

void MapEdit::Update()
{
	Point mousePos;
	if (GetMousePoint(&mousePos.x, &mousePos.y) == -1) {
		return;
	}
	// マウスの座標がマップエディタ領域内にいるかどうかを判定する
	isInMapEditArea_ = mousePos.x >= mapEditRect_.x && mousePos.x <= mapEditRect_.x + mapEditRect_.w && mousePos.y >= mapEditRect_.y && mousePos.y <= mapEditRect_.y + mapEditRect_.h;
	
	if (!isInMapEditArea_) return; //マップエディタ領域外なら何もしない

	int gridX = (mousePos.x - cfg_.LEFT_MARGIN) / cfg_.MAP_IMAGE_SIZE;
	int gridY = (mousePos.y - cfg_.TOP_MARGIN) / cfg_.MAP_IMAGE_SIZE;

	drawAreaRect_ = { cfg_.LEFT_MARGIN + gridX * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + gridY * cfg_.MAP_IMAGE_SIZE, cfg_.MAP_IMAGE_SIZE, cfg_.MAP_IMAGE_SIZE };

	if (Input::IsButtonKeep(MOUSE_INPUT_LEFT)) //左クリックでマップに値をセット
	{
		MapChip* mapChip = FindGameObject<MapChip>();

		if (CheckHitKey(KEY_INPUT_LSHIFT)) //Rキーを押しているなら
		{
			SetMap({ gridX + ScrollOffset_.x, gridY + ScrollOffset_.y }, -1); //マップに値をセット（-1は何もない状態）
			return; //マップチップを削除したらここで終了
		}
		else if (mapChip && mapChip->IsHold()) //マップチップを持っているなら
		{
			SetMap({ gridX + ScrollOffset_.x, gridY + ScrollOffset_.y }, mapChip->GetHoldImage()); //マップに値をセット
		}
	}

	if (Input::IsKeyDown(KEY_INPUT_LEFT))
		ScrollOffset_.x = std::max(0, ScrollOffset_.x - 1);
	if (Input::IsKeyDown(KEY_INPUT_RIGHT))
		ScrollOffset_.x = std::min(std::max(0, cfg_.MAP_WIDTH - cfg_.MAP_EDIT_VIEW_X), ScrollOffset_.x + 1);
	if (Input::IsKeyDown(KEY_INPUT_UP))
		ScrollOffset_.y = std::max(0, ScrollOffset_.y - 1);
	if (Input::IsKeyDown(KEY_INPUT_DOWN))
		ScrollOffset_.y = std::min(std::max(0, cfg_.MAP_HEIGHT - cfg_.MAP_EDIT_VIEW_Y), ScrollOffset_.y + 1);

	if (Input::IsKeyDown(KEY_INPUT_S))
	{
		SaveMapData();
	}
	if (Input::IsKeyDown(KEY_INPUT_L))
	{
		LoadMapData();
	}
	if (Input::IsKeyDown(KEY_INPUT_Q))
	{
		ClearMap();
	}
}

void MapEdit::Draw()
{
	for (int j = 0;j < cfg_.MAP_EDIT_VIEW_Y;j++)
	{
		for (int i = 0; i < cfg_.MAP_EDIT_VIEW_X; i++)
		{
			int gx = i + ScrollOffset_.x;
			int gy = j + ScrollOffset_.y;
			int value = GetMap({ i + gx,j + gy });
			if (value != -1) //-1なら何も描画しない
			{
				DrawGraph(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE, value, TRUE);
			}
		}
	}

	//for (int y = 0; y < cfg_.MAPCHIP_VIEW_Y; y++) {
	//	for (int x = 0; x < cfg_.TILES_X; x++) {

	//		int gx = x + ScrollOffset_.x;
	//		int gy = y + ScrollOffset_.y;
	//		int index = gy * cfg_.TILES_X + gx;
	//		DrawGraph(originX + x * cfg_.TILE_PIX_SIZE,
	//			originY + y * cfg_.TILE_PIX_SIZE,
	//			bgHandle[index], TRUE);
	//	}
	//}

	for (int j = 0;j < cfg_.MAP_EDIT_VIEW_Y;j++)
	{
		for (int i = 0; i < cfg_.MAP_EDIT_VIEW_X; i++)
		{
			assert(i >= 0 && i < cfg_.MAP_EDIT_VIEW_Y);
			assert(j >= 0 && j < cfg_.MAP_EDIT_VIEW_X);
			int gx = i + ScrollOffset_.x;
			int gy = j + ScrollOffset_.y;
			int value = selectedChip_[(j + gy)* cfg_.MAP_WIDTH + i + gx];
			if (value != -1) //-1なら何も描画しない
			{
				DrawGraph(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE, value, TRUE);
				DrawBox(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE, 					
					cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE + cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE + cfg_.MAP_IMAGE_SIZE,
					GetColor(255, 255, 255), TRUE);
			}
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 128);
	DrawBox(cfg_.LEFT_MARGIN + 0, cfg_.TOP_MARGIN + 0, cfg_.LEFT_MARGIN + cfg_.MAP_WIDTH * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + cfg_.MAP_HEIGHT * cfg_.MAP_IMAGE_SIZE, GetColor(255, 255, 0), FALSE, 5);
	for (int j = 0; j < cfg_.MAP_EDIT_VIEW_Y; j++) {
		for (int i = 0; i < cfg_.MAP_EDIT_VIEW_X; i++) {
			DrawLine(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE,
				cfg_.LEFT_MARGIN + (i + 1) * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE, GetColor(255, 255, 255), 1);
			DrawLine(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE,
				cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + (j + 1) * cfg_.MAP_IMAGE_SIZE, GetColor(255, 255, 255), 1);
		}
	}
	if (isInMapEditArea_) {

		DrawBox(drawAreaRect_.x, drawAreaRect_.y,
			drawAreaRect_.x + drawAreaRect_.w, drawAreaRect_.y + drawAreaRect_.h,
			GetColor(255, 255, 0), TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void MapEdit::SaveMapData()
{
	TCHAR filename[255] = "";
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFilter = "全てのファイル (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 255;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		printfDx("ファイルが選択された\n");
		std::ofstream openfile(filename);
		openfile << "#TinyMapData\n";

		MapChip* mc = FindGameObject<MapChip>();

		for (int j = 0; j < cfg_.MAP_HEIGHT; j++) {
			for (int i = 0; i < cfg_.MAP_WIDTH; i++) {

				int index;
				if (myMap_[j * cfg_.MAP_WIDTH + i] != -1)
					index = mc->GetChipIndex(myMap_[j * cfg_.MAP_WIDTH + i]);
				else
					index = -1;

				if (i == cfg_.MAP_WIDTH - 1) //最後の要素なら改行しない
				{
					openfile << index; //最後の要素はカンマをつけない
				}
				else
				{
					//最後の要素以外はカンマをつける
					openfile << index << ",";
				}
			}
			openfile << std::endl;
		}
		openfile.close();
		printfDx("File Saved!!!\n");
	}
}

void MapEdit::LoadMapData()
{
	TCHAR filename[255] = "";
	OPENFILENAME ifn = { 0 };

	ifn.lStructSize = sizeof(ifn);
	ifn.hwndOwner = GetMainWindowHandle();
	ifn.lpstrFilter = "全てのファイル (*.*)\0*.*\0";
	ifn.lpstrFile = filename;
	ifn.nMaxFile = 255;
	ifn.Flags = OFN_OVERWRITEPROMPT;

	if (GetOpenFileName(&ifn))
	{
		printfDx("ファイルが選択された→%s\n", filename);
		std::ifstream inputfile(filename);
		std::string line;

		MapChip* mc = FindGameObject<MapChip>();
		myMap_.clear();//マップを空に！
		while (std::getline(inputfile, line)) {
			if (line.empty()) continue;

			if (line[0] != '#')
			{
				std::istringstream iss(line);
				std::string tmp;//これに一個ずつ読み込んでいくよ
				while (getline(iss, tmp, ',')) {
					printfDx("%s ", tmp.c_str());
					if (tmp == "-1")
					{
						myMap_.push_back(-1); //何もない状態
					}
					else
					{
						int index = std::stoi(tmp);
						int handle = mc->GetHandle(index);
						myMap_.push_back(handle); //マップにハンドルをセット
					}

					
				}
				printfDx("\n");
			}
			//else
			//{
			//	MessageBox(nullptr, "ファイル形式が間違っています", "読み込みエラー", 
			//		MB_OK | MB_ICONWARNING);
			//}
		}
	}
	else
	{
		//ファイルの選択がキャンセル
		printfDx("ファイル選択がキャンセル\n");
	}
}

void MapEdit::ClearMap()
{
	for (int j = 0;j < cfg_.MAP_HEIGHT;j++)
	{
		for (int i = 0; i < cfg_.MAP_WIDTH; i++)
		{
			Point p = { i, j };
			SetMap(p , -1);
		}
	}
}


