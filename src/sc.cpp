
#include "sc.h"
#include<math.h>

using namespace cv;
using namespace std;

struct pixel{
    float initialE = 0;
    float optEnergy = 0;
    int optPreColIndex;
//    pixel *optPreLoc;
};

void delSeam(Mat& tempMat, Mat& new_image, pixel **curMat){//把最小seam所在pixel寻找到并删除，图片缩小存入new_image
    int rows = tempMat.rows;
    int cols = tempMat.cols;
    int lastRowIndex = rows-1;

    int tempMin = 0;
    float tempMinEnergy = curMat[lastRowIndex][0].optEnergy;
    for (int j = 1; j < cols; j++) {
        if(curMat[lastRowIndex][j].optEnergy < tempMinEnergy){
            tempMin = j;
            tempMinEnergy = curMat[lastRowIndex][j].optEnergy;
        }
    }

    int curColIndex = tempMin;
//    pixel *curloc = &curMat[lastRowIndex][tempMin];

    for (int j = 0; j < new_image.cols; j++) {//最后一行
        if(j < curColIndex){
            new_image.at<Vec3b>(lastRowIndex, j) = tempMat.at<Vec3b>(lastRowIndex, j);
        }else{
            new_image.at<Vec3b>(lastRowIndex, j) = tempMat.at<Vec3b>(lastRowIndex, j + 1);
        }
    }

    for (int i = lastRowIndex; i > 0; i--) {//最后一行以外
        for (int j = 0; j < new_image.cols; j++) {
            curColIndex = curMat[i][curColIndex].optPreColIndex;
            if(j < curColIndex){
                new_image.at<Vec3b>(i-1, j) = tempMat.at<Vec3b>(i-1, j);
            }else{
                new_image.at<Vec3b>(i-1, j) = tempMat.at<Vec3b>(i-1, j + 1);
            }
        }
    }

}

void optCalLeft(int& i, int& j , pixel **curMat){
    curMat[i][j].optEnergy = curMat[i-1][j-1].optEnergy + curMat[i][j].initialE;
    curMat[i][j].optPreColIndex = j - 1;
//    curMat[i][j].optPreLoc = &curMat[i-1][j-1];
}

void optCalRight(int& i, int& j , pixel **curMat){
    curMat[i][j].optEnergy = curMat[i-1][j+1].optEnergy + curMat[i][j].initialE;
    curMat[i][j].optPreColIndex = j + 1;
//    curMat[i][j].optPreLoc = &curMat[i-1][j+1];
}

void optCalMid(int& i, int& j , pixel **curMat){
    curMat[i][j].optEnergy = curMat[i-1][j].optEnergy + curMat[i][j].initialE;
    curMat[i][j].optPreColIndex = j;
//    curMat[i][j].optPreLoc = &curMat[i-1][j];
}

//能量方程公式 E= sqrt((up-down)^2 + (left-right)^2)
void calOptEnergy(Mat& tempMat,pixel **curMat,int& rows,int& cols){//计算每pixel能量最小值
    int lastCol = cols - 1;
    int lastRow= rows - 1;

    //得到灰度图
    Mat tempGrayMat;
    cvtColor(tempMat,tempGrayMat,CV_BGR2GRAY);

    for (int j = 0; j < cols; j++) {//第一行初始化

        if(j == 0){

            curMat[0][0].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(0,1)),2) + pow(((float)tempGrayMat.at<uchar>(1,0)),2));

        }else if(j == lastCol){

            curMat[0][lastCol].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(0,lastCol-1)),2) + pow(((float)tempGrayMat.at<uchar>(1,lastCol)),2));

        }else{

            curMat[0][j].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(0,j-1) - (float)tempGrayMat.at<uchar>(0,j+1)),2) + pow(((float)tempGrayMat.at<uchar>(1,j)),2));

        }

        curMat[0][j].optEnergy = curMat[0][j].initialE;

    }

    for (int i = 1; i < lastRow; i++) {//第二行到倒数第二行
        for (int j = 0; j < cols; j++) {

            if(j == 0){//第一列

                curMat[i][0].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(i,1)),2) + pow(((float)tempGrayMat.at<uchar>(i-1,0) - (float)tempGrayMat.at<uchar>(i+1,0)),2));

                if(curMat[i-1][j].optEnergy > curMat[i-1][j+1].optEnergy){//右边

                    optCalRight(i,j,curMat);

                }else{//中间

                    optCalMid(i,j,curMat);
                }


            }else if (j == lastCol){//最后一列

                curMat[i][lastCol].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(i,lastCol - 1)),2) + pow(((float)tempGrayMat.at<uchar>(i-1,lastCol) - (float)tempGrayMat.at<uchar>(i+1,lastCol)),2));

                if(curMat[i-1][j].optEnergy > curMat[i-1][j-1].optEnergy){//左边

                    optCalLeft(i,j,curMat);

                }else{//中间

                    optCalMid(i,j,curMat);
                }

            }else{

                curMat[i][j].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(i,j-1) - (float)tempGrayMat.at<uchar>(i,j+1)),2) + pow(((float)tempGrayMat.at<uchar>(i-1,j) - (float)tempGrayMat.at<uchar>(i+1,j)),2));

                float minEnergy = curMat[i-1][j].optEnergy;//中间
                optCalMid(i,j,curMat);

                if(minEnergy > curMat[i-1][j-1].optEnergy){//左边
                    minEnergy = curMat[i-1][j-1].optEnergy;
                    optCalLeft(i,j,curMat);
                }

                if(minEnergy > curMat[i-1][j+1].optEnergy){//右边
                    optCalRight(i,j,curMat);
                }
            }
        }

    }

    for(int j = 0; j < cols; j++){//最后一行
        if(j == 0){
            curMat[lastRow][0].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(lastRow,1)),2) + pow(((float)tempGrayMat.at<uchar>(lastRow - 1,0)),2));

            if(curMat[lastRow-1][j].optEnergy > curMat[lastRow-1][j+1].optEnergy){//右边

                optCalRight(lastRow,j,curMat);

            }else{//中间

                optCalMid(lastRow,j,curMat);
            }

        }else if (j == lastCol){
            curMat[lastRow][lastCol].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(lastRow,lastCol - 1)),2) + pow(((float)tempGrayMat.at<uchar>(lastRow - 1,lastCol)),2));

            if(curMat[lastRow-1][j].optEnergy > curMat[lastRow-1][j-1].optEnergy){//左边

                optCalLeft(lastRow,j,curMat);

            }else{//中间

                optCalMid(lastRow,j,curMat);
            }

        }else{

            curMat[tempGrayMat.rows-1][j].initialE = (float)sqrt(pow(((float)tempGrayMat.at<uchar>(tempGrayMat.rows-1,j-1) - (float)tempGrayMat.at<uchar>(tempGrayMat.rows-1,j+1)),2) + pow(((float)tempGrayMat.at<uchar>(tempGrayMat.rows-2,j)),2));

            float minEnergy = curMat[lastRow-1][j].optEnergy;//中间
            optCalMid(lastRow,j,curMat);

            if(minEnergy > curMat[lastRow-1][j-1].optEnergy){//左边
                minEnergy = curMat[lastRow-1][j-1].optEnergy;
                optCalLeft(lastRow,j,curMat);
            }

            if(minEnergy > curMat[lastRow-1][j+1].optEnergy){//右边
                optCalRight(lastRow,j,curMat);
            }
        }
    }

}

void operateOneSeam(Mat& tempMat){//删除一条seam操作

    int rows = tempMat.rows;
    int cols = tempMat.cols;

    pixel **curMat;
    curMat = new pixel *[rows];
    for( int i=0; i < rows; i++ ){
        curMat[i] = new pixel [cols]  ;
    }


    calOptEnergy(tempMat , curMat , rows,cols);

    Mat new_image(rows,cols-1,tempMat.type());
    delSeam(tempMat, new_image, curMat);

    for( int i=0; i < rows; i++ ){
        delete[] curMat[i];
    }
    delete[] curMat;

    tempMat = new_image;

}


bool seam_carving(Mat& in_image, int new_width, int new_height, Mat& out_image){

    // some sanity checks
    // Check 1 -> new_width <= in_image.cols
    if(new_width>in_image.cols){
        cout<<"Invalid request!!! new_width has to be smaller than the current size!"<<endl;
        return false;
    }
    if(new_height>in_image.rows){
        cout<<"Invalid request!!! ne_height has to be smaller than the current size!"<<endl;
        return false;
    }
    
    if(new_width<=0){
        cout<<"Invalid request!!! new_width has to be positive!"<<endl;
        return false;

    }
    
    if(new_height<=0){
        cout<<"Invalid request!!! new_height has to be positive!"<<endl;
        return false;
        
    }

    return seam_carving_trivial(in_image, new_width, new_height, out_image);
}


// seam carves by removing trivial seams
bool seam_carving_trivial(Mat& in_image, int new_width, int new_height, Mat& out_image){

    Mat tempMat = in_image.clone();

    while (tempMat.cols > new_width) {
//        cout<<"v: "<<tempMat.cols <<endl;
        operateOneSeam(tempMat);
    }

    //旋转图片tempMat，切横边
    transpose(tempMat,tempMat);
    flip(tempMat,tempMat,1);

    while(tempMat.cols > new_height) {
//        cout<<"h: "<< tempMat.cols <<endl;
        operateOneSeam(tempMat);
    }

    //再次旋转图片,恢复
    transpose(tempMat,tempMat);
    flip(tempMat,tempMat,0);

    out_image = tempMat;

    return true;
}
