//
//  branchScaleModel.h
//  OpenLung
//
//  Created by Felix Kratz on 08.07.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

#pragma once
#include "lung.h"

// The base class is a templated class and takes this class as a template argument,
// the inheritance will always look like this
class BinaryTreeLungBranch : public LungBranch<BinaryTreeLungBranch> {
    // Define the super type to easily access the base class
    typedef LungBranch<BinaryTreeLungBranch> super;

    // Use for additional setup of the branch at creation. The init() of the base class will call the init() of the branchParameter struct.
    // This is optional
    void init() override { }

public:
    // This is the initializer it will call the initializer of the base class
    // via the super type.
    BinaryTreeLungBranch(GlobalBranchParameters* _prm_global, BranchParameters prm) : super(_prm_global, prm) {
        init();
    };

    // This is the "local" time step for each of the branches
    inline void timeStep(ExternalParameters* _extern_params, LungParameters* _lung_params, TimeStepParameters* q) override
    {
        // Realization of the simple binary tree model on the branch level
        if ((_connections.size() == 0 || _connections[0]->_getBranchParameters()->isOpen) && !prm.isOpen)
        {
            // If the pressure difference is bigger than the pressure threshold
            // we change the volume based on the biomechanical equation that we came up with

            if (_lung_params->P - (_lung_params->P_ip) - prm.P_th > 0)
                prm.V += _prm_global->zeta * pow(prm.T,3) * prm.R * (_lung_params->P - (_lung_params->P_ip) - prm.P_th) * _extern_params->dt;

            // This makes sure that the volume of a branch always stays inside
            // of the volume bounds
            if (prm.V > prm.V_max)
            {
                prm.V = prm.V_max;
                prm.isOpen = true;
            }
        }
    }
};
