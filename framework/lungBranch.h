//
//  lungBranch.h
//  OpenLung
//
//  Created by Felix Kratz on 13.06.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

#pragma once
#include <vector>
#include <random>
#include "lung.h"
#include "model_params.h"

template <class Branch>
class LungBranch {
protected:
    // This holds the member variables, again: keep the member variables private
    // and use getters and setters instead for more control
    BranchParameters prm;
    GlobalBranchParameters* _prm_global;
    std::vector<Branch*> _connections;

    virtual void init() { prm.init(); };
public:
    // Getters for private variables (minimizing direct access)
    BranchParameters* _getBranchParameters() { return &prm; };
    GlobalBranchParameters* _getGlobalParameters() { return _prm_global; };
    std::vector<Branch*> getConnections() { return _connections; };
    Branch* _getConnection()
    {
        if (_connections.size() > 0)
            return _connections[0];
        else return nullptr;
    }

    // General setup of the branch with an abstract set of parameters defined in "model_params.h"
    LungBranch(GlobalBranchParameters* _prm_global, BranchParameters prm) : _prm_global(_prm_global), prm(prm) { init(); };

    // This adds the pointer of another branch to a vector of LungBranch pointers:
    // BEWARE OF MEMORY REALLOCATION! -> the _connections vector needs to have
    // enough memory allocated *before* pushing connections into it
    inline void addConnections(std::vector<Branch*> branches) { for (auto b : branches) { addConnection(b); }}
    void addConnection(Branch* _branch) { _connections.push_back(_branch); };

    // This deletes a connection between two lung branches
    void deleteConnection(Branch* _branch)
    {
        for (int i = 0; i < _connections.size(); i++)
        {
            if (_connections[i] == _branch)
            {
                _connections.erase(_connections.begin() + i);
                return;
            }
        };
    };

    // This deletes all connections of this branch
    void deleteConnections() { _connections.clear(); };

    // This has to be implemented for each individual model and makes this a
    // fully virtual class
    virtual inline void timeStep(ExternalParameters* _extern_params, LungParameters* _lung_params, TimeStepParameters* q) = 0;
};
