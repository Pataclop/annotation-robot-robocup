#include "traitement/position.h"

#include <hl_communication/utils.h>
#include <hl_monitoring/field.h>
#include <hl_monitoring/utils.h>
#include <hl_monitoring/monitoring_manager.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/calib3d.hpp>

using namespace hl_communication;
using namespace hl_monitoring;


namespace traitement
{
	Position::Position(){
		x = 0;
		y = 0;
		uncertainty = 0;

	}
	Position::~Position(){
	}

	void Position::setPosition(float xp, float yp ){
		x = xp;
		y = yp;
	}


}