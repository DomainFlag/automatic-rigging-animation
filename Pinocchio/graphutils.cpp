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

#include "graphutils.h"
#include "debugging.h"

#define CHECK(pred) { if(!(pred)) { Debugging::out() << "Graph integrity error: " #pred << " in line " << __LINE__ << endl; return false; } }

bool PtGraph::integrityCheck() const
{
    CHECK(verts.size() == edges.size());
    
    int i, j, k;
    for(i = 0; i < (int)edges.size(); ++i) {
        for(j = 0; j < (int)edges[i].size(); ++j) {
            int cur = edges[i][j];
            CHECK(cur >= 0);
            CHECK(cur < (int)edges.size());
            CHECK(cur != i); //no self edges
            
            vector<int>::const_iterator it = find(edges[cur].begin(), edges[cur].end(), i);
            CHECK(it != edges[cur].end());
            
            for(k = 0; k < j; ++k) //duplicates
                CHECK(cur != edges[i][k]);
        }
    }
    
    return true;
}

ShortestPather::ShortestPather(const PtGraph &g, int root)
{
    int sz = g.verts.size();
    priority_queue<Inf> todo;
    vector<bool> done(sz, false);
    prev.resize(sz, -1);
    dist.resize(sz, -1);
            
    todo.push(Inf(0., root, -1));
    while(!todo.empty())
    {
        Inf cur = todo.top();
        todo.pop();
        if(done[cur.node])
            continue;
        done[cur.node] = true;
        prev[cur.node] = cur.prev;
        dist[cur.node] = cur.dist;
                
        const vector<int> &e = g.edges[cur.node];
        for(int i = 0; i < (int)e.size(); ++i) {
            if(!done[e[i]]) {
                double dist = cur.dist + (g.verts[cur.node] - g.verts[e[i]]).length();
                todo.push(Inf(dist, e[i], cur.node));
            }
        }
    }
}
