#include "IVideoThreadThermalCam.h"
#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QString>


IVideoThreadThermalCam::IVideoThreadThermalCam(QObject *parent,
                                               QString _XML_File_Setting, QString _Name)
    : IVideoThread(parent,_Name)
{

    if(::evo_irimager_usb_init(_XML_File_Setting.toStdString().c_str(), 0, 0) != 0) return;

    if((err = ::evo_irimager_get_palette_image_size(&p_w, &p_h)) != 0)
    {
      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
      exit(-1);
    }

    if((err = ::evo_irimager_get_thermal_image_size(&t_w, &t_h)) != 0)
    {
      std::cerr << "error on evo_irimager_get_palette_image_size: " << err << std::endl;
      exit(-1);
    }

    palette_image.resize(p_w * p_h * 3);
    thermal_data.resize(t_w * t_h);


    m_Type = (TYPE_CAM::THERMAL);

    m_DispatcherControl.ThresholdDetection = 100;
    m_DispatcherControl.Mode = 2;
    m_DispatcherControl.isDetectionThermalArea = false;
    m_DispatcherControl.isBlurImage = false;
    m_DispatcherControl.isDrawCountur = false;
    m_DispatcherControl.isDrawArea = false;
    m_DispatcherControl.isInvertMask = false;
    m_DispatcherControl.isThermalPoint = false;

}

IVideoThreadThermalCam::~IVideoThreadThermalCam()
{
    isRun = false;
    exit(0);
    mVideoWriter.release();
    ::evo_irimager_terminate();

}


void IVideoThreadThermalCam::run()
{

    if(mVideoWriter.isOpened())
    {
      std::cout  << " \n ------------------------------- \n "
                    "start writer cpture video  |OK| \n"
                    " \n ------------------------------- \n ";
    }
    else
    {
        return;
    }



     while (isRun)
     {
         if((err = ::evo_irimager_get_palette_image(&p_w, &p_h, &palette_image[0]))==0)
         {

            ::evo_irimager_get_thermal_image(&t_w,&t_h,&thermal_data[0]);

           /**
           ::evo_irimager_get_thermal_image(&t_w,&t_h,&thermal_data[0]);
           unsigned long int mean = 0;
           //--Code for calculation mean temperature of image -----------------
           for (int y = 0; y < t_h; y++)
           {
             for (int x = 0; x < t_w; x++)
             {
               mean += thermal_data[y*t_w + x];
             }
           }
           **/


           int width = 640;
           int height = 480;

           //std::cout << p_w << " " << p_h << std::endl;
           //---------------------------------------------
           //--Code for displaying image -----------------
           cv::Mat cv_img(cv::Size(p_w, p_h), CV_8UC3, &palette_image[0], cv::Mat::AUTO_STEP);
           mFrame = cv_img;
           cv::cvtColor(mFrame, mFrame, cv::COLOR_BGR2RGB);
           cv::resize(mFrame, mFrame, cv::Size(width, height), cv::INTER_LANCZOS4);


           float unit_w = float(t_w) / float(width);
           float unit_h = float(t_h) / float(height);



           //----------------------------------------------------//

           if( m_DispatcherControl.isDetectionThermalArea )
           {

               cv::Mat img_gray;
               cv::cvtColor(mFrame, img_gray, cv::COLOR_BGR2GRAY);

               /**/

               int thresh = m_DispatcherControl.ThresholdDetection;
               int max_thresh = 255;
              // cv::RNG rng(12345);

               cv::Mat threshold_output;
               std::vector<std::vector<cv::Point> > contours;
               std::vector<cv::Vec4i> hierarchy;


               if( m_DispatcherControl.isBlurImage )
               {
                   // Define kernal for erosion and dilation and closing operations
                   cv::Mat kernel = cv::Mat(5, 5, CV_8U, cv::Scalar(1,1,1));
                   cv::blur( img_gray, img_gray, cv::Size(3,3) );
                   cv::erode(img_gray,img_gray,cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
                   cv::dilate(img_gray,img_gray,cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
                   cv::morphologyEx(img_gray, img_gray, cv::MORPH_CLOSE, kernel);
               }



               /// Detect edges using Threshold
               cv::threshold( img_gray, threshold_output, thresh, max_thresh, cv::THRESH_BINARY );

               /// Find contours
               cv::findContours( threshold_output, contours, hierarchy, CV_RETR_TREE,
                                 m_DispatcherControl.Mode, cv::Point(0, 0) );

               /// Approximate contours to polygons + get bounding rects and circles
               std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
               std::vector<cv::Rect> boundRect( contours.size() );
               std::vector<cv::Point2f>center( contours.size() );
               std::vector<float>radius( contours.size() );

               for( unsigned int i = 0; i < contours.size(); i++ )
               {
                   cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
                   boundRect[i] = boundingRect( cv::Mat(contours_poly[i]) );
                   minEnclosingCircle( (cv::Mat)contours_poly[i], center[i], radius[i] );
               }


               /// Draw polygonal contour + bonding rects + circles
               cv::Mat1b mask(mFrame.rows, mFrame.cols, uchar(0));
               //cv::Mat3b dbgRects = mFrame.clone();
               //cv::Mat drawing = cv::Mat::zeros( threshold_output.size(), CV_8UC3 );

               for( unsigned int i = 0; i< contours.size(); i++ )
               {
                   int real_w = ((boundRect[i].br().x  - boundRect[i].tl().x));
                   int real_h = ((boundRect[i].br().y  - boundRect[i].tl().y));

                   if( real_w < 20 || real_h < 20 ) continue;

                   cv::Scalar color = cv::Scalar( 0, 255, 0 );// rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
                   if(m_DispatcherControl.isDrawCountur)
                    cv::drawContours( mFrame, contours_poly, i, color, 1, 8, std::vector<cv::Vec4i>(), 0, cv::Point() );

                   if(m_DispatcherControl.isDrawArea)
                   cv::rectangle( mFrame, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
                   //cv::circle( mFrame, center[i], (int)radius[i], color, 2, 8, 0 );

                   //----------------------------------------------------------//


                   float mean = 0;
                   float C_Area = 0;
                   float C_Max = 0;
                   float max_tmp  = 0;


                   for(unsigned int j= 0; j < contours[i].size();j++) // run until j < contours[i].size();
                   {
                        cv::Point  p(contours[i][j]);
                        float temperature = (int)thermal_data[floor(p.x * unit_w) + floor(p.y * unit_h) * t_w];
                        mean += temperature;
                        if(max_tmp < temperature || j == 0)
                        {
                            max_tmp = temperature;
                        }

                   }

                   C_Area = (mean / contours[i].size()) / 10.0 - 100;
                   C_Max = (max_tmp) / 10.0 - 100;



//                   int ww = ((boundRect[i].br().x  - boundRect[i].tl().x)) * unit_w;
//                   int hh = ((boundRect[i].br().y  - boundRect[i].tl().y)) * unit_h;

//                   auto pp = boundRect[i].tl();// + (boundRect[i].br() - boundRect[i].tl()) * 0.5f;
//                   pp.x = pp.x * unit_w;
//                   pp.y = pp.y * unit_h;

//                   int mean = 0;
//                   int C_Area = 0;
//                   int C_Max = 0;
//                   int max_tmp  = -1000;
//                   if(pp.x < t_w && pp.y < t_h && ww > 0 && hh > 0)
//                   {
//                       for (int yy = pp.y; yy < pp.y + hh; yy++)
//                       {
//                           for (int xx = pp.x; xx < pp.x + ww; xx++)
//                           {
//                               int temperature = (int)thermal_data[yy*ww + xx];
//                               mean += temperature;

//                               if(max_tmp < temperature || max_tmp == -1000)
//                               {
//                                   max_tmp = temperature;
//                               }
//                           }
//                       }
//                       C_Area = (mean / (ww * hh)) / 10.0 - 100;
//                       C_Max = (max_tmp) / 10.0 - 100;
//                   }


                   //----------------------------------------------------------//


                   // Draw white rectangles on mask
                   cv::rectangle(mask, boundRect[i], cv::Scalar(255), CV_FILLED);

                   cv::putText(mFrame, //target image
                               QString::number(C_Area).toStdString(), //text
                               //"222",
                               boundRect[i].tl() + (boundRect[i].br() - boundRect[i].tl()) * 0.5f, //top-left position
                               cv::FONT_HERSHEY_DUPLEX,
                               0.5,
                               CV_RGB(0, 255, 0), //font color
                               2);


                   cv::putText(mFrame, //target image
                               QString::number(C_Max).toStdString(), //text
                               //"222",
                               (boundRect[i].tl() + (boundRect[i].br() - boundRect[i].tl()) * 0.5f) + cv::Point2i(0,15), //top-left position
                               cv::FONT_HERSHEY_DUPLEX,
                               0.5,
                               CV_RGB(0, 0, 255), //font color
                               2);

                   //----------------------------------------------------------//
               }


               if(m_DispatcherControl.isInvertMask)
               {
                   // Black initizlied result
                   cv::Mat3b result(mFrame.rows, mFrame.cols, cv::Vec3b(0,0,0));
                   mFrame.copyTo(result, mask);
                   mFrame = result;
               }

               /**/

           }

           //----------------------------------------------------//

           if( m_DispatcherControl.isThermalPoint )
           {
               cv::Point2i p( TickThermal.x / unit_w, TickThermal.y / unit_h);
               //cv::circle(mFrame,p,25,(0,0,255),2);

               cv::Point p1(0,10), p2(0,-10), p3(10,0), p4(-10,0);
               cv::Scalar colorLine(0,255,0); // Green
               int thicknessLine = 1;

               cv::line(mFrame, p, p+p1, colorLine, thicknessLine);
               cv::line(mFrame, p, p+p2, colorLine, thicknessLine);
               cv::line(mFrame, p, p+p3, colorLine, thicknessLine);
               cv::line(mFrame, p, p+p4, colorLine, thicknessLine);

               int temperature = (int)thermal_data[TickThermal.y*t_w + TickThermal.x];
               cv::putText(mFrame, //target image
                           QString::number(temperature / 10.0 - 100).toStdString(), //text
                           //"222",
                           cv::Point2i( TickThermal.x / unit_w, TickThermal.y / unit_h) + cv::Point2i(5,-15), //top-left position
                           cv::FONT_HERSHEY_DUPLEX,
                           0.5,
                           CV_RGB(0, 0, 255), //font color
                           2);
           }

           //---------------------------------------------------//

           if(!mFrame.empty())
           {
               if(mVideoWriter.isOpened())
               {
                   //std::cout << "writer send \n";
                   mVideoWriter << mFrame;
               }

               //mPixmap = cvMatToQPixmap(mFrame,isText,m_Name);
               //emit newPixmapCapture();
           }

         }
         else
         {
           std::cerr << "failed evo_irimager_get_thermal_palette_image: " << err << std::endl;
         }
     }
}




cv::VideoWriter &IVideoThreadThermalCam::VideoWriter()
{
    return mVideoWriter;
}

int IVideoThreadThermalCam::ThermalPoint(float x, float y)
{
    TickThermal.x = x;
    TickThermal.y = y;
     //::evo_irimager_get_thermal_image(&t_w,&t_h,&thermal_data[0]);
     int temperature = (int)thermal_data[y*t_w + x];
     return temperature / 10.0 - 100;;
}


