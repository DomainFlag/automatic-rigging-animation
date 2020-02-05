/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "skeleton.h"
#include "utils.h"
#include "debugging.h"
#include <fstream>
#include <unordered_map>

void Skeleton::initCompressed()
{
    int i;

    fcMapV.resize(fPrevV.size(), -1);
    fcFractionV.resize(fPrevV.size(), -1.);
    
    for(i = 0; i < (int)fPrevV.size(); ++i) {
		// Make sure to always include the root in the compressed skel!
        if(fGraphV.edges[i].size() == 2 && i != 0)
            continue;
        fcMapV[i] = cfMapV.size();
        cfMapV.push_back(i);
    }
    
    cPrevV.resize(cfMapV.size(), -1);
    cSymV.resize(cfMapV.size(), -1);
    cGraphV.edges.resize(cfMapV.size());
    cFeetV = vector<bool>(cPrevV.size(), false);
    cFatV = vector<bool>(cPrevV.size(), false);
    
    for(i = 0; i < (int)cfMapV.size(); ++i) {
        cGraphV.verts.push_back(fGraphV.verts[cfMapV[i]]);
        
        //symmetry--TODO: need to make sure all unreduced bones in chain
        //          are marked as symmetric before marking the reduced one
        if(fSymV[cfMapV[i]] >= 0)
            cSymV[i] = fcMapV[fSymV[cfMapV[i]]];
        
        //prev
        if(i > 0) {
            int curPrev = fPrevV[cfMapV[i]];
            while(fcMapV[curPrev]  < 0)
                curPrev = fPrevV[curPrev];
            cPrevV[i] = fcMapV[curPrev];
        }
    }
    
    //graph edges
    for(i = 1; i < (int)cPrevV.size(); ++i) {
        cGraphV.edges[i].push_back(cPrevV[i]);
        cGraphV.edges[cPrevV[i]].push_back(i);
    }
    
    cLengthV.resize(cPrevV.size(), 0.);

    //lengths/fraction computation
    for(i = 1; i < (int)cPrevV.size(); ++i) {
        int cur = cfMapV[i];
        unordered_map<int, double> lengths;
        do {
            lengths[cur] = (fGraphV.verts[cur] - fGraphV.verts[fPrevV[cur]]).length();
            cLengthV[i] += lengths[cur];
            cur = fPrevV[cur];
        } while(fcMapV[cur] == -1);
        
        for(unordered_map<int, double>::iterator it = lengths.begin(); it != lengths.end(); ++it)
            fcFractionV[it->first] = it->second / cLengthV[i];
    }
}

void Skeleton::scale(double factor)
{
    int i;
    for(i = 0; i < (int)fGraphV.verts.size(); ++i)
        fGraphV.verts[i] *= factor;
    for(i = 0; i < (int)cGraphV.verts.size(); ++i) {
        cGraphV.verts[i] *= factor;
        cLengthV[i] *= factor;
    }
}

void Skeleton::makeJoint(const string &name, const Pinocchio::Vector3 &pos, const string &previous)
{
    int cur = fSymV.size();
    fSymV.push_back(-1);
    fGraphV.verts.push_back(pos * 0.5); //skeletons specified in [-1,1] will be fit to object in [0,1]
    fGraphV.edges.resize(cur + 1);
    jointNames[name] = cur;
    joints.push_back(name);
    joints_names.insert({name, joints_names.size()});
    
    if(previous == string("")) {
        fPrevV.push_back(-1);
    } else { //add a bone
        int prev = jointNames[previous];
        fGraphV.edges[cur].push_back(prev);
        fGraphV.edges[prev].push_back(cur);
        fPrevV.push_back(prev);
    }
}

void Skeleton::makeSymmetric(const string &name1, const string &name2)
{
    int i1 = jointNames[name1];
    int i2 = jointNames[name2];

    if(i1 > i2)
        swap(i1, i2);
    fSymV[i2] = i1;
}

void Skeleton::setFoot(const string &name)
{
    int i = jointNames[name];
    cFeetV[fcMapV[i]] = true;
}

void Skeleton::setFat(const string &name)
{
    int i = jointNames[name];
    cFatV[fcMapV[i]] = true;
}

//-----------------actual skeletons-------------------

HumanSkeleton::HumanSkeleton()
{
    //order of makeJoint calls is very important
    makeJoint("hips",       Pinocchio::Vector3(0., 0., 0.));                           //0
    makeJoint("spine",      Pinocchio::Vector3(0., 0.15, 0.),      "hips");            //1
    makeJoint("shoulders",  Pinocchio::Vector3(0., 0.575, 0.),      "spine");          //2
    makeJoint("head",       Pinocchio::Vector3(0., 0.7, 0.),       "shoulders");       //3
    
    makeJoint("lshoulder",  Pinocchio::Vector3(-0.2, 0.5, 0.),     "shoulders");       //4
    makeJoint("lelbow",     Pinocchio::Vector3(-0.4, 0.25, 0.075), "lshoulder");       //5
    makeJoint("lwrist",     Pinocchio::Vector3(-0.6, 0.0, 0.15),   "lelbow");          //6
    makeJoint("lhand",      Pinocchio::Vector3(-0.6, -0.025, 0.15),"lwrist");          //7

    makeJoint("rshoulder",  Pinocchio::Vector3(0.2, 0.5, 0.),      "shoulders");       //8
    makeJoint("relbow",     Pinocchio::Vector3(0.4, 0.25, 0.075),  "rshoulder");       //9
    makeJoint("rwrist",     Pinocchio::Vector3(0.6, 0.0, 0.15),    "relbow");          //10
    makeJoint("rhand",      Pinocchio::Vector3(0.6, -0.025, 0.15), "rwrist");          //11
    
    makeJoint("lthigh",     Pinocchio::Vector3(-0.1, 0., 0.),      "hips");            //12
    makeJoint("lknee",      Pinocchio::Vector3(-0.15, -0.35, 0.),  "lthigh");          //13
    makeJoint("lankle",     Pinocchio::Vector3(-0.15, -0.8, 0.),   "lknee");           //14
    makeJoint("lfoot",      Pinocchio::Vector3(-0.15, -0.8, 0.1),  "lankle");          //15
    
    makeJoint("rthigh",     Pinocchio::Vector3(0.1, 0., 0.),       "hips");            //16
    makeJoint("rknee",      Pinocchio::Vector3(0.15, -0.35, 0.),   "rthigh");          //17
    makeJoint("rankle",     Pinocchio::Vector3(0.15, -0.8, 0.),    "rknee");           //18
    makeJoint("rfoot",      Pinocchio::Vector3(0.15, -0.8, 0.1),   "rankle");          //19
    
    //symmetry
    makeSymmetric("lthigh", "rthigh");
    makeSymmetric("lknee", "rknee");
    makeSymmetric("lankle", "rankle");
    makeSymmetric("lfoot", "rfoot");
    
    makeSymmetric("lshoulder", "rshoulder");
    makeSymmetric("lelbow", "relbow");
    makeSymmetric("lwrist", "rwrist");
    makeSymmetric("lhand", "rhand");

    initCompressed();

    setFoot("lfoot");
    setFoot("rfoot");

    setFat("hips");
    setFat("shoulders");
    setFat("head");
}

QuadSkeleton::QuadSkeleton()
{
    //order of makeJoint calls is very important
    makeJoint("shoulders",  Pinocchio::Vector3(0., 0., 0.5));
    makeJoint("back",       Pinocchio::Vector3(0., 0., 0.),         "shoulders");
    makeJoint("hips",       Pinocchio::Vector3(0., 0., -0.5),       "back");
    makeJoint("neck",       Pinocchio::Vector3(0., 0.2, 0.63),      "shoulders");
    makeJoint("head",       Pinocchio::Vector3(0., 0.2, 0.9),       "neck");
    
    makeJoint("lthigh",     Pinocchio::Vector3(-0.15, 0., -0.5),     "hips");
    makeJoint("lhknee",     Pinocchio::Vector3(-0.2, -0.4, -0.5),   "lthigh");
    makeJoint("lhfoot",     Pinocchio::Vector3(-0.2, -0.8, -0.5),   "lhknee");
    
    makeJoint("rthigh",     Pinocchio::Vector3(0.15, 0., -0.5),      "hips");
    makeJoint("rhknee",     Pinocchio::Vector3(0.2, -0.4, -0.5),    "rthigh");
    makeJoint("rhfoot",     Pinocchio::Vector3(0.2, -0.8, -0.5),    "rhknee");
    
    makeJoint("lshoulder",  Pinocchio::Vector3(-0.2, 0., 0.5),      "shoulders");
    makeJoint("lfknee",     Pinocchio::Vector3(-0.2, -0.4, 0.5),    "lshoulder");
    makeJoint("lffoot",      Pinocchio::Vector3(-0.2, -0.8, 0.5),   "lfknee");
    
    makeJoint("rshoulder",  Pinocchio::Vector3(0.2, 0.0, 0.5),      "shoulders");
    makeJoint("rfknee",     Pinocchio::Vector3(0.2, -0.4, 0.5),     "rshoulder");
    makeJoint("rffoot",      Pinocchio::Vector3(0.2, -0.8, 0.5),    "rfknee");
    
    makeJoint("tail",       Pinocchio::Vector3(0., 0., -0.7),       "hips");
    
    //symmetry
    makeSymmetric("lthigh", "rthigh");
    makeSymmetric("lhknee", "rhknee");
    makeSymmetric("lhfoot", "rhfoot");
    
    makeSymmetric("lshoulder", "rshoulder");
    makeSymmetric("lfknee", "rfknee");
    makeSymmetric("lffoot", "rffoot");
    
    initCompressed();

    setFoot("lhfoot");
    setFoot("rhfoot");
    setFoot("lffoot");
    setFoot("rffoot");

    setFat("hips");
    setFat("shoulders");
    setFat("head");
}

HorseSkeleton::HorseSkeleton()
{
    //order of makeJoint calls is very important
    makeJoint("shoulders",  Pinocchio::Vector3(0., 0., 0.5));
    makeJoint("back",       Pinocchio::Vector3(0., 0., 0.),         "shoulders");
    makeJoint("hips",       Pinocchio::Vector3(0., 0., -0.5),       "back");
    makeJoint("neck",       Pinocchio::Vector3(0., 0.2, 0.63),      "shoulders");
    makeJoint("head",       Pinocchio::Vector3(0., 0.2, 0.9),       "neck");
    
    makeJoint("lthigh",     Pinocchio::Vector3(-0.15, 0., -0.5),     "hips");
    makeJoint("lhknee",     Pinocchio::Vector3(-0.2, -0.2, -0.45),  "lthigh");
    makeJoint("lhheel",     Pinocchio::Vector3(-0.2, -0.4, -0.5),   "lhknee");
    makeJoint("lhfoot",     Pinocchio::Vector3(-0.2, -0.8, -0.5),   "lhheel");
    
    makeJoint("rthigh",     Pinocchio::Vector3(0.15, 0., -0.5),      "hips");
    makeJoint("rhknee",     Pinocchio::Vector3(0.2, -0.2, -0.45),   "rthigh");
    makeJoint("rhheel",     Pinocchio::Vector3(0.2, -0.4, -0.5),    "rhknee");
    makeJoint("rhfoot",     Pinocchio::Vector3(0.2, -0.8, -0.5),    "rhheel");
    
    makeJoint("lshoulder",  Pinocchio::Vector3(-0.2, 0., 0.5),      "shoulders");
    makeJoint("lfknee",     Pinocchio::Vector3(-0.2, -0.4, 0.5),    "lshoulder");
    makeJoint("lffoot",      Pinocchio::Vector3(-0.2, -0.8, 0.5),   "lfknee");
    
    makeJoint("rshoulder",  Pinocchio::Vector3(0.2, 0.0, 0.5),      "shoulders");
    makeJoint("rfknee",     Pinocchio::Vector3(0.2, -0.4, 0.5),     "rshoulder");
    makeJoint("rffoot",      Pinocchio::Vector3(0.2, -0.8, 0.5),    "rfknee");
    
    makeJoint("tail",       Pinocchio::Vector3(0., 0., -0.7),       "hips");
    
    //symmetry
    makeSymmetric("lthigh", "rthigh");
    makeSymmetric("lhknee", "rhknee");
    makeSymmetric("lhheel", "rhheel");
    makeSymmetric("lhfoot", "rhfoot");
    
    makeSymmetric("lshoulder", "rshoulder");
    makeSymmetric("lfknee", "rfknee");
    makeSymmetric("lffoot", "rffoot");
    
    initCompressed();

    setFoot("lhfoot");
    setFoot("rhfoot");
    setFoot("lffoot");
    setFoot("rffoot");

    setFat("hips");
    setFat("shoulders");
    setFat("head");
}

CentaurSkeleton::CentaurSkeleton()
{
    //order of makeJoint calls is very important
    makeJoint("shoulders",  Pinocchio::Vector3(0., 0., 0.5));                      //0
    makeJoint("back",       Pinocchio::Vector3(0., 0., 0.),         "shoulders");  //1
    makeJoint("hips",       Pinocchio::Vector3(0., 0., -0.5),       "back");       //2

    makeJoint("hback",      Pinocchio::Vector3(0., 0.25, 0.5),      "shoulders");  //3
    makeJoint("hshoulders", Pinocchio::Vector3(0., 0.5, 0.5),       "hback");      //4
    makeJoint("head",       Pinocchio::Vector3(0., 0.7, 0.5),       "hshoulders"); //5
    
    makeJoint("lthigh",     Pinocchio::Vector3(-0.15, 0., -0.5),    "hips");       //6
    makeJoint("lhknee",     Pinocchio::Vector3(-0.2, -0.4, -0.45),  "lthigh");     //7
    makeJoint("lhfoot",     Pinocchio::Vector3(-0.2, -0.8, -0.5),   "lhknee");     //8
    
    makeJoint("rthigh",     Pinocchio::Vector3(0.15, 0., -0.5),     "hips");       //9
    makeJoint("rhknee",     Pinocchio::Vector3(0.2, -0.4, -0.45),   "rthigh");     //10
    makeJoint("rhfoot",     Pinocchio::Vector3(0.2, -0.8, -0.5),    "rhknee");     //11
    
    makeJoint("lshoulder",  Pinocchio::Vector3(-0.2, 0., 0.5),      "shoulders");  //12
    makeJoint("lfknee",     Pinocchio::Vector3(-0.2, -0.4, 0.5),    "lshoulder");  //13
    makeJoint("lffoot",     Pinocchio::Vector3(-0.2, -0.8, 0.5),    "lfknee");     //14
    
    makeJoint("rshoulder",  Pinocchio::Vector3(0.2, 0.0, 0.5),      "shoulders");  //15
    makeJoint("rfknee",     Pinocchio::Vector3(0.2, -0.4, 0.5),     "rshoulder");  //16
    makeJoint("rffoot",     Pinocchio::Vector3(0.2, -0.8, 0.5),     "rfknee");     //17
    
    makeJoint("hlshoulder", Pinocchio::Vector3(-0.2, 0.5, 0.5),     "hshoulders"); //18
    makeJoint("lelbow",     Pinocchio::Vector3(-0.4, 0.25, 0.575),  "hlshoulder"); //19
    makeJoint("lhand",      Pinocchio::Vector3(-0.6, 0.0, 0.65),    "lelbow");     //20
    
    makeJoint("hrshoulder", Pinocchio::Vector3(0.2, 0.5, 0.5),      "hshoulders"); //21
    makeJoint("relbow",     Pinocchio::Vector3(0.4, 0.25, 0.575),   "hrshoulder"); //22
    makeJoint("rhand",      Pinocchio::Vector3(0.6, 0.0, 0.65),     "relbow");     //23

    makeJoint("tail",       Pinocchio::Vector3(0., 0., -0.7),       "hips");       //24

    //symmetry
    makeSymmetric("lthigh", "rthigh");
    makeSymmetric("lhknee", "rhknee");
    makeSymmetric("lhheel", "rhheel");
    makeSymmetric("lhfoot", "rhfoot");
    
    makeSymmetric("lshoulder", "rshoulder");
    makeSymmetric("lfknee", "rfknee");
    makeSymmetric("lffoot", "rffoot");

    makeSymmetric("hlshoulder", "hrshoulder");
    makeSymmetric("lelbow", "relbow");
    makeSymmetric("lhand", "rhand");    
    
    initCompressed();

    setFoot("lhfoot");
    setFoot("rhfoot");
    setFoot("lffoot");
    setFoot("rffoot");

    setFat("hips");
    setFat("shoulders");
    setFat("hshoulders");
    setFat("head");
}

FileSkeleton::FileSkeleton(const std::string &filename)
{
    ifstream strm(filename.c_str());
  
    if(!strm.is_open()) {
        Debugging::out() << "Error opening file " << filename << endl;
        return;
    }

    while(!strm.eof()) {
        vector<string> line = readWords(strm);
        if(line.size() < 5)
            continue; //error

        Pinocchio::Vector3 p;
        sscanf(line[1].c_str(), "%lf", &(p[0]));
        sscanf(line[2].c_str(), "%lf", &(p[1]));
        sscanf(line[3].c_str(), "%lf", &(p[2]));

        if(line[4] == "-1")
            line[4] = std::string();

        makeJoint(line[0], p * 2., line[4]);
    }

    initCompressed();
}
