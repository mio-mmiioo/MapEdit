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

MapEdit::MapEdit()
	:GameObject(), cfg_(GetMapEditConfig()), myMap_(cfg_.MAP_WIDTH* cfg_.MAP_HEIGHT, -1),
	selectedChip_(cfg_.MAP_WIDTH* cfg_.MAP_HEIGHT, -1),
	copyChips_(cfg_.MAP_WIDTH* cfg_.MAP_HEIGHT, -1),
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
	return myMap_[p.y * cfg_.MAP_WIDTH + p.x]; //y�sx��̒l���擾����

}

void MapEdit::Update()
{
	Point mousePos;
	if (GetMousePoint(&mousePos.x, &mousePos.y) == -1) {
		return;
	}
	// �}�E�X�̍��W���}�b�v�G�f�B�^�̈���ɂ��邩�ǂ����𔻒肷��
	isInMapEditArea_ = mousePos.x >= mapEditRect_.x && mousePos.x <= mapEditRect_.x + mapEditRect_.w &&
		mousePos.y >= mapEditRect_.y && mousePos.y <= mapEditRect_.y + mapEditRect_.h;

	//����@mapEditRect_.x, mapEditRect_.y
	//�E��@mapEditRect_.x + mapEditRect_.w, mapEditRect_.y
	//�E��  mapEditRect_.x + mapEditRect_.w, mapEditRect_.y + mapEditRect_.h
	//����  mapEditRect_.x, mapEditRect_.y + mapEditRect_.h
		// �O���b�h���W�ɕϊ�
	if (!isInMapEditArea_) {
		return; //�}�b�v�G�f�B�^�̈�O�Ȃ牽�����Ȃ�
	}

	int gridX = (mousePos.x - cfg_.LEFT_MARGIN) / cfg_.MAP_IMAGE_SIZE;
	int gridY = (mousePos.y - cfg_.TOP_MARGIN) / cfg_.MAP_IMAGE_SIZE;

	drawAreaRect_ = { cfg_.LEFT_MARGIN + gridX * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + gridY * cfg_.MAP_IMAGE_SIZE,
		cfg_.MAP_IMAGE_SIZE, cfg_.MAP_IMAGE_SIZE };

	if (Input::IsButtonKeep(MOUSE_INPUT_LEFT)) //���N���b�N�Ń}�b�v�ɒl���Z�b�g
	{
		MapChip* mapChip = FindGameObject<MapChip>();

		if (CheckHitKey(KEY_INPUT_LSHIFT)) //R�L�[�������Ă���Ȃ�
		{
			SetMap({ gridX, gridY }, -1); //�}�b�v�ɒl���Z�b�g�i-1�͉����Ȃ���ԁj
			return; //�}�b�v�`�b�v���폜�����炱���ŏI��
		}
		else if (mapChip && mapChip->IsHold()) //�}�b�v�`�b�v�������Ă���Ȃ�
		{
			SetMap({ gridX, gridY }, mapChip->GetHoldImage()); //�}�b�v�ɒl���Z�b�g
		}

	}

	CopyMapData({ gridX, gridY }, mousePos);

	
		if (Input::IsKeyDown(KEY_INPUT_S))
	{
		SaveMapData();
	}
	if (Input::IsKeyDown(KEY_INPUT_L))
	{
		LoadMapData();
	}
}

void MapEdit::Draw()
{//�w�i��`�悷��

	for (int j = 0;j < cfg_.MAP_HEIGHT;j++)
	{
		for (int i = 0; i < cfg_.MAP_WIDTH; i++)
		{
			int value = GetMap({ i,j });
			if (value != -1) //-1�Ȃ牽���`�悵�Ȃ�
			{
				DrawGraph(cfg_.LEFT_MARGIN + i * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + j * cfg_.MAP_IMAGE_SIZE,
					value, TRUE);
			}
		}
	}

	for (int j = 0;j < cfg_.MAP_HEIGHT;j++)
	{
		for (int i = 0; i < cfg_.MAP_WIDTH; i++)
		{
			assert(i >= 0 && i < cfg_.MAP_WIDTH);
			assert(j >= 0 && j < cfg_.MAP_HEIGHT);
			int value = selectedChip_[j * cfg_.MAP_WIDTH + i];
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
	DrawBox(cfg_.LEFT_MARGIN + 0, cfg_.TOP_MARGIN + 0,
		cfg_.LEFT_MARGIN + cfg_.MAP_WIDTH * cfg_.MAP_IMAGE_SIZE, cfg_.TOP_MARGIN + cfg_.MAP_HEIGHT * cfg_.MAP_IMAGE_SIZE, GetColor(255, 255, 0), FALSE, 5);
	for (int j = 0; j < cfg_.MAP_HEIGHT; j++) {
		for (int i = 0; i < cfg_.MAP_WIDTH; i++) {
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
	//�撣���ăt�@�C���I���_�C�A���O���o����
	TCHAR filename[255] = "";
	OPENFILENAME ofn = { 0 };

	ofn.lStructSize = sizeof(ofn);
	//�E�B���h�E�̃I�[�i�[���e�E�B���h�E�̃n���h��
	ofn.hwndOwner = GetMainWindowHandle();
	ofn.lpstrFilter = "�S�Ẵt�@�C�� (*.*)\0*.*\0";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = 255;
	ofn.Flags = OFN_OVERWRITEPROMPT;


	if (GetSaveFileName(&ofn))
	{
		printfDx("�t�@�C�����I�����ꂽ\n");
		//�t�@�C�����J���āA�Z�[�u
		//std::filesystem �t�@�C�����������o��
		//ofstream���J��
		std::ofstream openfile(filename);
		//�t�@�C���̑I�����L�����Z��
		//printfDx("�Z�[�u���L�����Z��\n");
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
	//�撣���ăt�@�C���I���_�C�A���O���o����
	TCHAR filename[255] = "";
	OPENFILENAME ifn = { 0 };

	ifn.lStructSize = sizeof(ifn);
	//�E�B���h�E�̃I�[�i�[���e�E�B���h�E�̃n���h��
	ifn.hwndOwner = GetMainWindowHandle();
	ifn.lpstrFilter = "�S�Ẵt�@�C�� (*.*)\0*.*\0";
	ifn.lpstrFile = filename;
	ifn.nMaxFile = 255;
	ifn.Flags = OFN_OVERWRITEPROMPT;

	if (GetOpenFileName(&ifn))
	{
		printfDx("�t�@�C�����I�����ꂽ��%s\n", filename);
		//�t�@�C�����J���āA�Z�[�u
		//std::filesystem �t�@�C�����������o��
		//ifstream���J�� input file stream
		std::ifstream inputfile(filename);
		//�t�@�C�����I�[�v���������ǂ����̓`�F�b�N���K�v
		std::string line;

		//�}�b�v�`�b�v�̏�����肽���I
		MapChip* mc = FindGameObject<MapChip>();
		myMap_.clear();//�}�b�v����ɁI
		while (std::getline(inputfile, line)) {
			// ��s�̓X�L�b�v
			if (line.empty()) continue;
			//printfDx("%s\n", line.c_str());
			//�����ɁA�ǂݍ��݂̏����������Ă����I
			if (line[0] != '#')
			{
				std::istringstream iss(line);
				std::string tmp;//����Ɉ���ǂݍ���ł�����
				while (getline(iss, tmp, ',')) {
					//if(tmp == -1)
					//	myMap_.push_back( -1);
					//else
					//	myMap_.push_back(mc->GetHandle(tmp)); //�}�b�v�Ƀn���h�����Z�b�g
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

void MapEdit::CopyMapData(Point p, Point mouse)
{
	if (Input::IsKeepKeyDown(KEY_INPUT_LCONTROL))
	{
		if (myMap_[p.x, p.y] != -1)
		{
			selectedChip_[p.x, p.y] = GetMap(p);
		}

		if (Input::IsKeyDown(KEY_INPUT_C))
		{
			copyChips_ = selectedChip_;
			selectedChip_ = { cfg_.MAP_WIDTH * cfg_.MAP_HEIGHT, -1 };
		}

		if (Input::IsKeyDown(KEY_INPUT_V))
		{

		}
	}
}


