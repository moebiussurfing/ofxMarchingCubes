/* 
 * Copyright (c) 2009, Rui Madeira
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * http://creativecommons.org/licenses/LGPL/2.1/
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include "ofxMarchingCubes.h"

//#define or ||

ofxMarchingCubes::ofxMarchingCubes(){
	
}

ofxMarchingCubes::~ofxMarchingCubes(){
	clear();
}

void ofxMarchingCubes::init(const glm::vec3& _iniGridPos,const glm::vec3& _gridSize, unsigned int _gridResX, unsigned int _gridResY,unsigned int _gridResZ){
	iniGridPos = _iniGridPos;
	gridResX = MAX(2, _gridResX);
	gridResY = MAX(2, _gridResY);
	gridResZ = MAX(2, _gridResZ);
	gridSize = _gridSize;
	setupGrid();
}

void ofxMarchingCubes::clear(){
	for(int i=0; i<gridResX; i++){
		for(int j=0; j<gridResY; j++){
			isoValues[i][j].clear();
			gridPoints[i][j].clear();
		}
		isoValues[i].clear();
		gridPoints[i].clear();
	}
	isoValues.clear();
	gridPoints.clear();
	numTriangles = 0;
}

void ofxMarchingCubes::update(float _threshold, bool bCalcNormals){
	threshold = _threshold;
	numTriangles = 0;
	vertices.clear(); 
	normals.clear();
	int numPointsX = gridResX-1;;
	int numPointsY = gridResY-1;
	int numPointsZ = gridResZ-1;
	for(unsigned int i=0; i<numPointsX; ++i){
		for(unsigned int j=0; j<numPointsY; ++j){
			for(unsigned int k=0; k<numPointsZ; ++k){
				polygonise(i, j, k, bCalcNormals);
			}
		}
	}
}	

void ofxMarchingCubes::debugDraw(){
	glColor3f(1.0f, 1.0f, 1.0f);
	drawFilled();
	glLineWidth(2.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	drawWireFrame();
	drawCube();
	//drawGrid();
}

void ofxMarchingCubes::drawWireFrame(){
	int numVertices = vertices.size();
	normals.resize(numVertices);
	for(int i=0; i<numVertices; i+=3){
		glBegin(GL_LINE_STRIP);
		glNormal3f(normals[i].x, normals[i].y, normals[i].z);
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
		glVertex3f(vertices[i+1].x, vertices[i+1].y, vertices[i+1].z);
		glVertex3f(vertices[i+2].x, vertices[i+2].y, vertices[i+2].z);
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
		glEnd();
	}
}

void ofxMarchingCubes::drawFilled(){
	int numVertices = vertices.size();
	normals.resize(numVertices);
	glBegin(GL_TRIANGLES);
	for(int i=0; i<numVertices; i+=3){
		glNormal3f(normals[i].x, normals[i].y, normals[i].z);
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
		glVertex3f(vertices[i+1].x, vertices[i+1].y, vertices[i+1].z);
		glVertex3f(vertices[i+2].x, vertices[i+2].y, vertices[i+2].z);
	}
	glEnd();
}

void ofxMarchingCubes::drawCube(){
	glPushMatrix();
	glTranslatef(iniGridPos.x, iniGridPos.y, iniGridPos.z);
	glScalef(gridSize.x, gridSize.y, gridSize.z);
    ofPushStyle();
    ofNoFill();
    ofDrawBox(1.0f);
    ofPopStyle();
	glPopMatrix();
}

void ofxMarchingCubes::drawGrid(){
	for(int i=0; i<gridResX; i++){
		for(int j=0; j<gridResY; j++){
			glBegin(GL_LINE_STRIP);
			for(int k=0; k<gridResZ; k++){
				glm::vec3& p = gridPoints[i][j][k];
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
		}
	}
	for(int i=0; i<gridResZ; i++){
		for(int j=0; j<gridResY; j++){
			glBegin(GL_LINE_STRIP);
			for(int k=0; k<gridResX; k++){
				glm::vec3& p = gridPoints[k][j][i];
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
		}
	}
	for(int i=0; i<gridResZ; i++){
		for(int j=0; j<gridResX; j++){
			glBegin(GL_LINE_STRIP);
			for(int k=0; k<gridResY; k++){
				glm::vec3& p = gridPoints[j][k][i];
				glVertex3f(p.x, p.y, p.z);
			}
			glEnd();
		}
	}
}

void ofxMarchingCubes::addMetaBall(const glm::vec3& pos, float force){
	float distSQ;
	glm::vec3 diff;
	for(int i=0; i<gridResX; i++){
		for(int j=0; j<gridResY; j++){
			for(int k=0; k<gridResZ; k++){
				diff = gridPoints[i][j][k] - pos;
				isoValues[i][j][k] += force / sqrtf(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);
			}
		}
	}
}

void ofxMarchingCubes::setIsoValue(unsigned int gridX, unsigned int gridY, unsigned int gridZ, float value){
	if(gridX >= gridResX || gridY >= gridResY || gridZ >= gridResZ){
		ofLog(OF_LOG_WARNING, "ofxMarchingCubes::setIsoValues - grid index out of bounds");
		return;
	}
	isoValues[gridX][gridY][gridZ] = value;
}

float ofxMarchingCubes::getIsoValue(unsigned int gridX, unsigned int gridY, unsigned int gridZ){
	if(gridX >= gridResX || gridY >= gridResY || gridZ >= gridResZ){
		ofLog(OF_LOG_WARNING, "ofxMarchingCubes::getIsoValues - grid index out of bounds, returning zero");
		return 0;
	}
	return isoValues[gridX][gridY][gridZ];
}

void ofxMarchingCubes::resetIsoValues(){
	for(int i=0; i<gridResX; i++){
		for(int j=0; j<gridResY; j++){
			for(int k=0; k<gridResZ; k++){
				isoValues[i][j][k] = 0.0f;
			}
		}
	}
}

float ofxMarchingCubes::getMaxIsoValue(){
	if(!gridResX || !gridResY || !gridResZ) return 0.0f;
	float max = isoValues[0][0][0];
	float val;
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=1; k<gridResZ; ++k){
				val = isoValues[i][j][k];
				if(val > max){
					max = val;
				}
			}
		}
	}
	return max;
}

float ofxMarchingCubes::getMinIsoValue(){
	if(!gridResX || !gridResY || !gridResZ) return 0.0f;
	float min = isoValues[0][0][0];
	float val;
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=1; k<gridResZ; ++k){
				val = isoValues[i][j][k];
				if(val < min){
					min = val;
				}
			}
		}
	}
	return min;
}

float ofxMarchingCubes::getAverageIsoValue(){
	if(!gridResX || !gridResY || !gridResZ) return 0.0f;
	float avg = isoValues[0][0][0];
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=1; k<gridResZ; ++k){
				avg += isoValues[i][j][k];
			}
		}
	}
	avg /= gridResX * gridResY * gridResZ;
	return avg;
}

void ofxMarchingCubes::scaleIsoValues(float amount){
	if(!gridResX || !gridResY || !gridResZ) return;
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=0; k<gridResZ; ++k){
				isoValues[i][j][k] *= amount;
			}
		}
	}
}

void ofxMarchingCubes::shiftIsoValues(float amount){
	if(!gridResX || !gridResY || !gridResZ) return;
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=0; k<gridResZ; ++k){
				isoValues[i][j][k] += amount;
			}
		}
	}
}

void ofxMarchingCubes::normalizeIsoValues(){
	if(!gridResX || !gridResY || !gridResZ) return;
	float min = getMinIsoValue();
	if(min < 0.0f){
		min = -min;
		shiftIsoValues(min);
	}
	float scale = getMaxIsoValue();
	scale = 1.0f / scale;
	scaleIsoValues(scale);
}

void ofxMarchingCubes::rescaleIsoValues(float min, float max){
	if(!gridResX || !gridResY || !gridResZ) return;
	min = MIN(min, max);
	max = MAX(min, max);
	float diff = max - min;
	normalizeIsoValues();
	scaleIsoValues(diff);
	shiftIsoValues(min);
}

void ofxMarchingCubes::absoluteValues(){
	if(!gridResX || !gridResY || !gridResZ) return;
	for(int i=0; i<gridResX; ++i){
		for(int j=0; j<gridResY; ++j){
			for(int k=1; k<gridResZ; ++k){
				isoValues[i][j][k] = ABS(isoValues[i][j][k]);
			}
		}
	}
}

int ofxMarchingCubes::getNumTriangles(){
	return numTriangles;	
}

float ofxMarchingCubes::getThreshold(){
	return threshold;	
}

vector<glm::vec3>& ofxMarchingCubes::getVertices(){
	return vertices;	
}

vector<glm::vec3>& ofxMarchingCubes::getNormals(){
	return normals;	
}

ofxMCGridValues& ofxMarchingCubes::getIsoValues(){
	return isoValues;
}

ofxMCGridPoints& ofxMarchingCubes::getGrid(){
	return gridPoints;	
}

void ofxMarchingCubes::setGridPos(const glm::vec3& _gridPos){
	if(_gridPos.x == iniGridPos.x && _gridPos.y == iniGridPos.y && _gridPos.z == iniGridPos.z) return;
	
}

void ofxMarchingCubes::setGridSize(const glm::vec3& _gridSize){
	if(_gridSize.x == gridSize.x && _gridSize.y == gridSize.y && _gridSize.z == gridSize.z)return;
	clear();
	gridSize = _gridSize;
	setupGrid();
}

void ofxMarchingCubes::setGridRes(unsigned int _gridResX, unsigned int _gridResY, unsigned int _gridResZ){
	if(_gridResX == gridResX && _gridResY == gridResY && _gridResZ == gridResZ)return;
	clear();
	gridResX = MAX(2, _gridResX);
	gridResY = MAX(2, _gridResY);
	gridResZ = MAX(2, _gridResZ);
	setupGrid();
}

glm::vec3 ofxMarchingCubes::getGridPos(){
	return iniGridPos;
}

glm::vec3 ofxMarchingCubes::getGridSize(){
	return gridSize;	
}

glm::vec3 ofxMarchingCubes::getGridRes(){
	return glm::vec3(gridResX, gridResY, gridResZ);
}

void ofxMarchingCubes::saveModel(string fileName, bool bUseASCII_mode){
	stlExporter.useASCIIFormat(bUseASCII_mode);
	
	int numVertices = vertices.size();
	for(int i=0; i<numVertices; i+=3){
		stlExporter.addTriangle(vertices[i], vertices[i+1], vertices[i+2], normals[i]);
	}
	stlExporter.saveModel(fileName);
}

ofxSTLExporter& ofxMarchingCubes::getSTLExporter(){
	return stlExporter;	
}

void ofxMarchingCubes::setupGrid(){
	glm::vec3 gap(gridSize.x / (gridResX-1), gridSize.y / (gridResY-1), gridSize.z / (gridResZ-1));
	glm::vec3 gridShift = gridSize * -0.5f;
	gridPoints.resize(gridResX);
	for(int i=0; i<gridResX; ++i){
		gridPoints[i].resize(gridResY);
		for(int j=0; j<gridResY; ++j){
			gridPoints[i][j].resize(gridResZ);
			for(int k=0; k<gridResZ; ++k){
				gridPoints[i][j][k] = glm::vec3(i*gap.x, j*gap.y, k*gap.z);
				gridPoints[i][j][k] += gridShift;
				gridPoints[i][j][k] += iniGridPos;
			}
		}
	}
	
	isoValues.resize(gridResX);
	for(int i=0; i<gridResX; i++){
		isoValues[i].resize(gridResY);
		for(int j=0; j<gridResY; j++){
			isoValues[i][j].resize(gridResZ);
			for(int k=0; k<gridResZ; k++){
				isoValues[i][j][k] = 0;
			}
		}
	}
	numTriangles = 0;	
}

void ofxMarchingCubes::vertexInterp(float threshold,const glm::vec3& p1,const glm::vec3& p2, float valp1, float valp2, glm::vec3& theVertice){
	float mu;
	if (ABS(threshold-valp1) < 0.00001){
		theVertice = glm::vec3(p1.x, p1.y, p1.z);
		return;
	}
	if (ABS(threshold-valp2) < 0.00001){
		theVertice = glm::vec3(p2.x, p2.y, p2.z);
		return;
	}
	if (ABS(valp1-valp2) < 0.00001){
		theVertice = glm::vec3(p1.x, p1.x, p1.z);
		return;
	}
	mu = (threshold - valp1) / (valp2 - valp1);
	theVertice.x = p1.x + mu * (p2.x - p1.x);
	theVertice.y = p1.y + mu * (p2.y - p1.y);
	theVertice.z = p1.z + mu * (p2.z - p1.z);
}

void ofxMarchingCubes::polygonise(unsigned int i, unsigned int j, unsigned int k, bool bCalcNormals){
	int cubeindex;
	
	cubeindex = 0;
	if (isoValues[i][j][k] < threshold) cubeindex |= 1;
	if (isoValues[i+1][j][k] < threshold) cubeindex |= 2;
	if (isoValues[i+1][j+1][k] < threshold) cubeindex |= 4;
	if (isoValues[i][j+1][k] < threshold) cubeindex |= 8;
	if (isoValues[i][j][k+1] < threshold) cubeindex |= 16;
	if (isoValues[i+1][j][k+1] < threshold) cubeindex |= 32;
	if (isoValues[i+1][j+1][k+1] < threshold) cubeindex |= 64;
	if (isoValues[i][j+1][k+1] < threshold) cubeindex |= 128;
	if (edgeTable[cubeindex] == 0){
	return;
	}
	
	if (edgeTable[cubeindex] & 1){
		vertexInterp(threshold, gridPoints[i][j][k], gridPoints[i+1][j][k], isoValues[i][j][k] ,isoValues[i+1][j][k], vertList[0]);
	}
	if (edgeTable[cubeindex] & 2){
		vertexInterp(threshold, gridPoints[i+1][j][k], gridPoints[i+1][j+1][k], isoValues[i+1][j][k], isoValues[i+1][j+1][k], vertList[1]);
	}
	if (edgeTable[cubeindex] & 4){
		 vertexInterp(threshold, gridPoints[i+1][j+1][k], gridPoints[i][j+1][k], isoValues[i+1][j+1][k], isoValues[i][j+1][k], vertList[2]);
	}
	if (edgeTable[cubeindex] & 8){
		 vertexInterp(threshold, gridPoints[i][j+1][k], gridPoints[i][j][k], isoValues[i][j+1][k], isoValues[i][j][k], vertList[3]);
	}
	if (edgeTable[cubeindex] & 16){
		 vertexInterp(threshold, gridPoints[i][j][k+1], gridPoints[i+1][j][k+1], isoValues[i][j][k+1], isoValues[i+1][j][k+1], vertList[4]);
	}
	if (edgeTable[cubeindex] & 32){
		 vertexInterp(threshold, gridPoints[i+1][j][k+1], gridPoints[i+1][j+1][k+1], isoValues[i+1][j][k+1], isoValues[i+1][j+1][k+1], vertList[5]);
	}
	if (edgeTable[cubeindex] & 64){
		vertexInterp(threshold, gridPoints[i+1][j+1][k+1], gridPoints[i][j+1][k+1], isoValues[i+1][j+1][k+1], isoValues[i][j+1][k+1], vertList[6]);
	}
	if (edgeTable[cubeindex] & 128){
		vertexInterp(threshold, gridPoints[i][j+1][k+1], gridPoints[i][j][k+1], isoValues[i][j+1][k+1], isoValues[i][j][k+1], vertList[7]); 
	}
	if (edgeTable[cubeindex] & 256){
		vertexInterp(threshold, gridPoints[i][j][k], gridPoints[i][j][k+1], isoValues[i][j][k], isoValues[i][j][k+1], vertList[8]);
	}
	if (edgeTable[cubeindex] & 512){
		vertexInterp(threshold, gridPoints[i+1][j][k], gridPoints[i+1][j][k+1], isoValues[i+1][j][k], isoValues[i+1][j][k+1], vertList[9]); 
	}
	if (edgeTable[cubeindex] & 1024){
		vertexInterp(threshold, gridPoints[i+1][j+1][k], gridPoints[i+1][j+1][k+1], isoValues[i+1][j+1][k], isoValues[i+1][j+1][k+1], vertList[10]); 
	}
	if (edgeTable[cubeindex] & 2048){
		vertexInterp(threshold,	gridPoints[i][j+1][k], gridPoints[i][j+1][k+1], isoValues[i][j+1][k], isoValues[i][j+1][k+1], vertList[11]); 
	}
	for (i=0;triTable[cubeindex][i]!=-1;i+=3) {
		if(bCalcNormals){
			glm::vec3 a = vertList[triTable[cubeindex][i+1]] - vertList[triTable[cubeindex][i]];
			glm::vec3 b = vertList[triTable[cubeindex][i+2]] - vertList[triTable[cubeindex][i+1]];
			//cross product - a cross b
			glm::vec3 normalVec = glm::cross(a, b);
            normalVec = glm::normalize(normalVec);
			normals.push_back(normalVec);
			normals.push_back(normalVec);
			normals.push_back(normalVec);
		}
		vertices.push_back(glm::vec3(vertList[triTable[cubeindex][i]].x, vertList[triTable[cubeindex][i]].y, vertList[triTable[cubeindex][i]].z));
		vertices.push_back(glm::vec3(vertList[triTable[cubeindex][i+1]].x, vertList[triTable[cubeindex][i+1]].y, vertList[triTable[cubeindex][i+1]].z));
		vertices.push_back(glm::vec3(vertList[triTable[cubeindex][i+2]].x, vertList[triTable[cubeindex][i+2]].y, vertList[triTable[cubeindex][i+2]].z));
		numTriangles++;
	}	
}






