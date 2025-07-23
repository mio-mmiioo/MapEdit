#pragma once

struct MapEditConfig {
	int MAP_WIDTH;
	int MAP_HEIGHT;
	int MAP_IMAGE_SIZE;
	int LEFT_MARGIN;
	int TOP_MARGIN;
};

const MapEditConfig& GetMapEditConfig();
