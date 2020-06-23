//
//  lungScaleModel.h
//  OpenLung
//
//  Created by Felix Kratz on 08.07.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

#pragma once
#include "lung.h"

// The base class is a templated class and takes the Branch class as a template argument
class BinaryTreeLung : public Lung<BinaryTreeLungBranch> {
private:
    typedef Lung<BinaryTreeLungBranch> super;

    // This is an override to the vitual member function init() of the base class, the base classes init() will still be called
    // and it will take care of all the memory management. So this is purely for further, model specific, setup.
    // Here we set our initial conditions for the simulation
    void init() override {
        lungPrm.P = externPrm.P_init;
        lungPrm.P_ip = externPrm.P_ip_init;
        lungPrm.V_ip = externPrm.V_ip_init;
    }

    void updateVolume() {
        // This updates the volume of the lung by adding up all the volumes of the branches
        lungPrm.V = 0;
        for (int i = 0; i < branches.size(); i++) lungPrm.V += branches[i]._getBranchParameters()->V;
    }

public:
    // These are the initializers they will call the initializer of the base class
    BinaryTreeLung(GlobalBranchParameters branch_prm, ExternalParameters extern_prm) : super(branch_prm, extern_prm) { init(); };
    BinaryTreeLung(ExternalParameters extern_prm) : super(extern_prm) { init(); };

    // This is the "global" time step for the lung
    // Realization of the simple binary tree model on the lung level
    inline void timeStep(TimeStepParameters* q) override
    {
        // Volume increase in the thoraxic cavity
        double oldV_ip = lungPrm.V_ip;
        lungPrm.V_ip = q->V_ip;

        // Pressure change caused by the volume increase in the thoraxic cavity
        lungPrm.P_ip = (lungPrm.P_ip + 1)* oldV_ip / lungPrm.V_ip - 1.;

        double oldV = lungPrm.V;

        // Update the volume of the alveolar branches
        // Only use asyncUpdateBranches(q) if you are certain, that the model
        // supports asynchronous updates.
        updateBranches(q);


        // Calculate new volume from the new volume of all the branches
        updateVolume();

        // Pressure change in the alveoli caused by the volume change
        lungPrm.P = (lungPrm.P + 1) * oldV / lungPrm.V - 1.;

        // Calculation of the effective airway resistance based on the current opening of the lung
        // It seems that the airflow resistance of the layers that we are modeling
        // is not relevant so we will use an airflow resistance only for the
        // static parts of our model, this can be adapted easily by
        // calculating the *dynamic* airflow resistance by considering all
        // the branches.
        double omega = externPrm.Omega + externPrm.dt / lungPrm.V;

        // Airflow into the alveoli caused by the pressure change in the alveoli.
        lungPrm.q = - (lungPrm.P)/omega;

        // This saves the intermediate pressure change into the lung parameters
        // so it is saved via a history event and can be plotted
        // The intermediate pressure change is only a theoretical construct and
        // has no physical meaning
        lungPrm.dP = lungPrm.P;

        // Flow into the lungs equillibrates pressure
        lungPrm.P += lungPrm.q*externPrm.dt / lungPrm.V;

        // A history event will only create a history entry based on the historyTrackingRate.
        // This behaviour can be overriden by simply setting the historyTrackingRate = 1
        // and only calling triggerHistoryEvent() when an entry should be created
        triggerHistoryEvent();
    };

    // Get the N-th branch down the line, only important for some special figures
    BinaryTreeLungBranch* _getConnectionOfOrder(int N, BinaryTreeLungBranch* branch)
    {
        if (N == 0) return branch;
        return _getConnectionOfOrder(N-1, branch->_getConnection());
    }
};
