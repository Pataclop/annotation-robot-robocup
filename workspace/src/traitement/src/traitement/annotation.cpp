#include "traitement/annotation.h"


#include <hl_communication/utils.h>
#include <hl_monitoring/monitoring_manager.h>
#include <hl_monitoring/utils.h>
#include <hl_monitoring/field.h>
#include <traitement/position.h>
#include <traitement/direction.h>


#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <tclap/CmdLine.h>
#include <string>
#include <sstream>
#include <iostream>
#include <json/value.h>
#include <jsoncpp/json/json.h>
#include <fstream>

#define SIZE_CIRCLE_POS 10
#define SIZE_ARROW 50
#define SIZE_CIRCLE_TRACE 5



using namespace hl_communication;
using namespace hl_monitoring;


namespace traitement{
  Annotation::Annotation(std::string){
    Json::Reader reader;	
    Json::Value root;

    std::ifstream annotation_settings("annotation_settings.json");
    annotation_settings >> root;

	    
    annotation_choice["position"]=root["position"]["write"].asBool();
    annotation_choice["direction"]=root["direction"]["write"].asBool();
    annotation_choice["trace"]=root["trace"]["write"].asBool();
	     
    sizecircle = root["position"]["circle_size"].asUInt();
    sizecircletrace = root["trace"]["circle_size"].asUInt();
    sizearrow = root["direction"]["arrow_size"].asUInt();
    robottrace = root["trace"]["robot_num"].asUInt();
	  
    cv::Scalar color1 = {root["color_team_1"]["r"].asUInt(), root["color_team_1"]["g"].asUInt(), root["color_team_1"]["b"].asUInt()};
    
    cv::Scalar color2 = {root["color_team_2"]["r"].asUInt(), root["color_team_2"]["g"].asUInt(), root["color_team_2"]["b"].asUInt()};

    color[root["color_team_1"]["num"].asUInt()]=color1;
    color[root["color_team_2"]["num"].asUInt()]=color2;
  }



  Annotation::~Annotation(){
  }


  cv::Mat  Annotation::annotePosition(Position pos, CameraMetaInformation camera_information, RobotInformation rb ,cv::Mat display){
    cv :: Point3f pos_in_field(pos.x, pos.y, 0.0);
    cv :: Point2f pos_in_img = fieldToImg(pos_in_field, camera_information);
    // cv::Point2f pos_in_img(pos.x, pos.y);
    if (color.find(rb.getTeam())!=color.end())
      cv::circle(display,pos_in_img, sizecircle,  color[rb.getTeam()],cv::FILLED);
    else
      cv::circle(display,pos_in_img, sizecircle, cv::Scalar(0,0,0),cv::FILLED);
		
    return display;
  }
  cv::Mat  Annotation::annoteDirection(Position pos, Direction dir, CameraMetaInformation camera_information, RobotInformation rb,cv::Mat display){
    // calcul pos
    cv :: Point3f pos_in_field(pos.x, pos.y, 0.0);
    cv :: Point2f pos_in_img = fieldToImg(pos_in_field, camera_information);
    //calcul dir
    cv :: Point3f pos_in_fielddir(pos.x+cos(dir.mean), pos.y+sin(dir.mean), 0.0);
    cv :: Point2f pos_in_imgdir = fieldToImg(pos_in_fielddir, camera_information);
    /* reduction taille des flèches à une longueur de 50 pour que la taille des flèches soit homogène*/
    float hypo = sqrt((pos_in_imgdir.x - pos_in_img.x)*(pos_in_imgdir.x - pos_in_img.x) +(pos_in_imgdir.y- pos_in_img.y)*(pos_in_imgdir.y- pos_in_img.y));
    cv :: Point2f fleche;
    fleche.x =  pos_in_img.x + (sizearrow*(pos_in_imgdir.x - pos_in_img.x)/hypo);
    fleche.y= pos_in_img.y + (sizearrow*(pos_in_imgdir.y- pos_in_img.y)/hypo);
    /*Affichage couleur pour les angles de degrès bizarre*/
    if (dir.mean > 2*CV_PI) 
      cv :: arrowedLine(display, pos_in_img, fleche, cv::Scalar(0,0,0), 2, 0, 0.1);
    else
      if (color.find(rb.getTeam())!=color.end())
	cv :: arrowedLine(display, pos_in_img, fleche, color[rb.getTeam()], 2, 0, 0.1);
      else
	cv :: arrowedLine(display, pos_in_img, fleche, cv::Scalar(0,0,0), 2, 0, 0.1);
    
    return display;
  }

  
  cv::Mat  Annotation::annoteTrace(Position pos, Direction dir, CameraMetaInformation camera_information, RobotInformation rb,cv::Mat display){
    int qsize = rb.sizeOfQueue();
    for (int i = 0; i<qsize; i++){
      Position p;
      p = rb.getPosition();
      //      display = annotePosition(p, camera_information, rb, display);
      cv :: Point3f pos_in_field(p.x, p.y, 0.0);
      cv :: Point2f pos_in_img = fieldToImg(pos_in_field, camera_information);
      cv::circle(display,pos_in_img, sizecircletrace, cv::Scalar(0,0,0),cv::FILLED); 
    }
    return display;
 
  }

  cv::Mat Annotation::AddAnnotation(Position pos,Direction dir, CameraMetaInformation camera_information, RobotInformation rb ,cv::Mat display){
    if (annotation_choice["position"])
      display = annotePosition(pos, camera_information, rb , display);
    if (annotation_choice["direction"])
      display = annoteDirection(pos, dir, camera_information, rb, display);
    if (annotation_choice["trace"] && rb.getNumRobotInformation() == robottrace){
      display = annoteTrace(pos, dir, camera_information, rb, display);
    }
    return display;
  }

	
}
/* on crée un monitoring, on load des messages, on appelle les classes pour afficher des annotations, ex = position, fleche etc */
