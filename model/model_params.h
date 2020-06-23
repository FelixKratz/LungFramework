//
//  model_params_pressureControl.h
//  OpenLung
//
//  Created by Felix Kratz on 08.07.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

struct TimeStepParameters {
    double V_ip;
};

// Parameters given by the specific realization
struct ExternalParameters {
    double dt;
    double P_init;
    double P_ip_init;
    double V_ip_init;
    double Omega;
    int N_branches;
};

// Parameters that apply to all branches
struct GlobalBranchParameters {
    double zeta;
};

// Parameters that are individual for every branch
struct BranchParameters {
    // Branch scale quantities
    double R;
    double L;
    double T;
    double V;
    double V_max;
    double P_th;
    short layer_ID;
    bool isOpen = false;
    bool isStatic = false;

    void validateVolume() {
        if (V >= M_PI * pow(R, 2) * L)
        {
            V = M_PI * pow(R, 2) * L;
            isOpen = true;
        }
        else if (V <= 0)  { V = 0; isOpen = false; }
        else isOpen = false;
    };

    void setMaxVolume() {
        if (!isStatic)
            V_max = M_PI * pow(R, 2) * L;
        else
        {
            V_max = V;
            isOpen = true;
        }
    };

    void init() {
        if (!isStatic)
            validateVolume();
        setMaxVolume();
    };
};

// Parameters that define the global behaviour of the lung
struct LungParameters {
    bool inhale = true;
    // Lung scale quantities parameters
    double P = 0;
    double V = 0;
    double dP = 0;
    double q = 0;

    double V_max = 0;
    double V_min = 0;

    // Intrapleural parameters
    double P_ip = 0;
    double V_ip = 0;

    void addBranchAdjustments(BranchParameters* _prm) {
        // Adjust the volume, maxVolume and minVolume to accomodate for the addition of another branch
        V += _prm->V;
        V_max += _prm->V_max;
        if (_prm->isStatic) V_min += _prm->V;
    }

    void removeBranchAdjustments(BranchParameters* _prm) {
        // Adjust the volume and maxVolume to accomodate for the removal of the branch
        V -= _prm->V;
        V_max -= _prm->V_max;
        if (_prm->isStatic) V_min -= _prm->V;
    }
};
