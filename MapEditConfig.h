#pragma once

struct MapEditConfig {
	int MAP_WIDTH;//マップの全長
	int MAP_HEIGHT;
	int MAP_EDIT_VIEW_X;//見える範囲
	int MAP_EDIT_VIEW_Y;
	int MAP_IMAGE_SIZE;
	int LEFT_MARGIN;
	int TOP_MARGIN;
};

const MapEditConfig& GetMapEditConfig();
