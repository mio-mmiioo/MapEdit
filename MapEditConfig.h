#pragma once

struct MapEditConfig {
	int MAP_WIDTH;//�}�b�v�̑S��
	int MAP_HEIGHT;
	int MAP_EDIT_VIEW_X;//������͈�
	int MAP_EDIT_VIEW_Y;
	int MAP_IMAGE_SIZE;
	int LEFT_MARGIN;
	int TOP_MARGIN;
};

const MapEditConfig& GetMapEditConfig();
