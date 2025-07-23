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
	isInMapEditArea_(false) //�}�b�v�G�f�B�^�̈���ɂ��邩�ǂ���
{
	mapEditRect_ = { cfg_.LEFT_MARGIN, cfg_.TOP_MARGIN, cfg_.MAP_WIDTH * cfg_.MAP_IMAGE_SIZE, cfg_.MAP_HEIGHT * cfg_.MAP_IMAGE_SIZE };
}

MapEdit::~MapEdit()
{
}

void MapEdit::SetMap(Point p, int value)
{
	//�}�b�v�̍��Wp��value���Z�b�g����
	//p���A�z��͈̔͊O�̎���assert�ɂЂ�������
	assert(p.x >= 0 && p.x < cfg_.MAP_WIDTH);
	assert(p.y >= 0 && p.y < cfg_.MAP_HEIGHT);
	myMap_[p.y * cfg_.MAP_WIDTH + p.x] = value; //y�sx���value���Z�b�g����
}

int MapEdit::GetMap(Point p) const
{
	//�}�b�v�̍��Wp�̒l���擾����
	//p���A�z��͈̔͊O�̎���assert�ɂЂ�������
	assert(p.x >= 0 && p.x < cfg_.MAP_WIDTH);
	assert(p.y >= 0 && p.y < cfg_.MAP_HEIGHT);
	//assert(p.x >= 0 && p.x < cfg_.MAP_EDIT_VIEW_X + ScrollOffset_.x);
	//assert(p.y >= 0 && p.y < cfg_.MAP_EDIT_VIEW_Y + ScrollOffset_.y);
	return myMap_[p.y * cfg_.MAP_WIDTH + p.x]; //y�sx��̒l���擾����
}

void MapEdit::Update()
{
	Point mousePos;
	if (GetMousePoint(&mousePos.x, &mousePos.y) == -1) {
		return;
	}
	// �}�E�X�̍��W���}�b�v�G�f�B�^�̈���ɂ��邩�ǂ����𔻒肷��
	isInMapEditArea_ = mousePos.x >= mapEditRect_.x && mousePos.x <= mapEditRect_.x + mapEditRect_.w && mousePos.y >= mapEditRect_.y && mousePos.y <= mapEditRect_.y + mapEditRect_.h;
	
	if (!isInMapEditArea_) return; //�}�b�v�G�f�B�^�̈�O�Ȃ牽�����Ȃ�

	int gridX = (mousePos.x - cfg_.LEFT_MARGIN) / cfg_.MAP_IMAGE_SIZE;
	int gridY = (mousePos.y - cfg_.TOP_MARGIN) / cfg_.MAP_IMAGE_SIZE;

	drawAreaRect_ = { cfg_.LEFT_MARGIN + gridX * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + gridY * cfg_.MAP_IMAGE_SIZE, cfg_.MAP_IMAGE_SIZE, cfg_.MAP_IMAGE_SIZE };

	if (Input::IsButtonKeep(MOUSE_INPUT_LEFT)) //���N���b�N�Ń}�b�v�ɒl���Z�b�g
	{
		MapChip* mapChip = FindGameObject<MapChip>();

		if (CheckHitKey(KEY_INPUT_LSHIFT)) //R�L�[�������Ă���Ȃ�
		{
			SetMap({ gridX + ScrollOffset_.x, gridY + ScrollOffset_.y }, -1); //�}�b�v�ɒl���Z�b�g�i-1�͉����Ȃ���ԁj
			return; //�}�b�v�`�b�v���폜�����炱���ŏI��
		}
		else if (mapChip && mapChip->IsHold()) //�}�b�v�`�b�v�������Ă���Ȃ�
		{
			SetMap({ gridX + ScrollOffset_.x, gridY + ScrollOffset_.y }, mapChip->GetHoldImage()); //�}�b�v�ɒl���Z�b�g
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
			if (value != -1) //-1�Ȃ牽���`�悵�Ȃ�
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
			if (value != -1) //-1�Ȃ牽���`�悵�Ȃ�
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
	ofn.lpstrFilter = "�S�Ẵt�@�C�� (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 255;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
	{
		printfDx("�t�@�C�����I�����ꂽ\n");
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

				if (i == cfg_.MAP_WIDTH - 1) //�Ō�̗v�f�Ȃ���s���Ȃ�
				{
					openfile << index; //�Ō�̗v�f�̓J���}�����Ȃ�
				}
				else
				{
					//�Ō�̗v�f�ȊO�̓J���}������
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
	ifn.lpstrFilter = "�S�Ẵt�@�C�� (*.*)\0*.*\0";
	ifn.lpstrFile = filename;
	ifn.nMaxFile = 255;
	ifn.Flags = OFN_OVERWRITEPROMPT;

	if (GetOpenFileName(&ifn))
	{
		printfDx("�t�@�C�����I�����ꂽ��%s\n", filename);
		std::ifstream inputfile(filename);
		std::string line;

		MapChip* mc = FindGameObject<MapChip>();
		myMap_.clear();//�}�b�v����ɁI
		while (std::getline(inputfile, line)) {
			if (line.empty()) continue;

			if (line[0] != '#')
			{
				std::istringstream iss(line);
				std::string tmp;//����Ɉ���ǂݍ���ł�����
				while (getline(iss, tmp, ',')) {
					printfDx("%s ", tmp.c_str());
					if (tmp == "-1")
					{
						myMap_.push_back(-1); //�����Ȃ����
					}
					else
					{
						int index = std::stoi(tmp);
						int handle = mc->GetHandle(index);
						myMap_.push_back(handle); //�}�b�v�Ƀn���h�����Z�b�g
					}

					
				}
				printfDx("\n");
			}
			//else
			//{
			//	MessageBox(nullptr, "�t�@�C���`�����Ԉ���Ă��܂�", "�ǂݍ��݃G���[", 
			//		MB_OK | MB_ICONWARNING);
			//}
		}
	}
	else
	{
		//�t�@�C���̑I�����L�����Z��
		printfDx("�t�@�C���I�����L�����Z��\n");
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


