//
//  lung.h
//  OpenLung
//
//  Created by Felix Kratz on 13.06.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

#pragma once

#include <vector>
#include <thread>
#include "lungBranch.h"

// This is the general lung framework that will be the same for every lung model

// TODO: removing a branch will mess with the connections, think about how to do it.
// TODO: Think of a good way to visualize the system

struct FunctionalParameters {
    bool detailedHistory = false; // This will take up much RAM if used with a big lung network
    bool globalParamsEnabled = false;
    int useMultithreading = 1;
    int historyTrackingRate = 0;
    int timeSteps = 0;
};

typedef std::pair<LungParameters, std::vector<BranchParameters>> HistoryPair;

template <class Branch>
class Lung {
protected:
    // This is where all the member variables are located. It is wise to protect
    // them from direct access (set them private) and to use getters/setters for access
    LungParameters lungPrm;
    ExternalParameters externPrm;
    GlobalBranchParameters branchPrmGlobal;
    FunctionalParameters functionalPrm;

    std::vector<Branch> branches;
    std::vector<HistoryPair> history;

    // virtual protected member functions
    virtual void init() {
        // This will allocate enough memory for all branches. If this is set to
        // the wrong size the vector will reallocate in memory and all the
        // connections become invalid! So it is better to reserve more space
        // than needed!
        branches.reserve(externPrm.N_branches);
    };

    virtual inline void updateBranches(TimeStepParameters* q) {
        for (int i = (int)branches.size() - 1; i >= 0; i--)
            branches[i].timeStep(&externPrm, &lungPrm, q);
    }

    // TODO: There is an error with the order of operations in mt which can cause
    // the simulation to behave strangely
    virtual inline void asyncUpdateBranches(TimeStepParameters* q) {
        // This calls all the threads for updating the branches, if multithreading
        // is off: only one thread will be created and this function should not be used.
        if (functionalPrm.useMultithreading <= 0) {
          std::cout << "Threading not enabled! Setting up threading with 4 threads..." << std::endl;
          functionalPrm.useMultithreading = 4;
        }
        std::vector<std::thread> threads(functionalPrm.useMultithreading);
        for (int i = 0; i < functionalPrm.useMultithreading; i++)
        {
            // Currently the threads take care of the Nth part of the branch count.
            // This might not be the best partitioning for asymetrical branch updates.
            // TODO: Should this be adjustable?
            threads[i] = std::thread(&Lung::partialTimeStep, this, int(i * double(branches.size())/double(functionalPrm.useMultithreading)), int((i + 1) * double(branches.size())/double(functionalPrm.useMultithreading)) , q);
        }
        // Join the threads before leaving this function, otherwise nothing is
        // guaranteed to work as intended.
        for (int i = 0; i < functionalPrm.useMultithreading; i++) {
          threads[i].join();
        }
    };

    inline void partialTimeStep(int N_min, int N_max, TimeStepParameters* q) {
        for (int i = N_min; i < N_max; i++) branches[i].timeStep(&externPrm, &lungPrm, q);
    };

    inline void triggerHistoryEvent() {
        // Record the changes made in this time step and write it into the history vector
        // This will only trigger every -historyTrackingRate- steps to be easier on the memory
        // If -historyTrackingRate- = 0 the history will not be created at all (default)
        functionalPrm.timeSteps++;
        if (functionalPrm.timeSteps % functionalPrm.historyTrackingRate != 0) return;

        std::vector<BranchParameters> branchParams;
        if (functionalPrm.detailedHistory) branchParams.reserve(branches.size());

        // This will create a deep copy of the branch parameters
        if (functionalPrm.detailedHistory)
            for (int i = 0; i < branches.size(); i++) branchParams.push_back(*branches[i]._getBranchParameters());

        // This emplaces copies of the objects into the history vector.
        // Don't work with pointers here!
        history.emplace_back(lungPrm, branchParams);
    }

public:
    LungParameters* _getLungParams() { return &lungPrm; };
    std::vector<HistoryPair>* _getHistory() { return &history; };

    void trackHistory(int trackingRate = 1, bool detailed = false) {
        functionalPrm.historyTrackingRate = trackingRate;
        functionalPrm.detailedHistory = detailed;
    };

    // Only use threading if you can make sure that your branch update is
    // independent of other branch updates!
    void useThreading(int threadCount = 4) {
        functionalPrm.useMultithreading = threadCount;
    };

    // These are the initializers for the class. If there is a need of additional
    // steps at creation: use the init() function of the derived model class for that.
    Lung(GlobalBranchParameters branch_prm, ExternalParameters extern_prm) : branchPrmGlobal(branch_prm), externPrm(extern_prm) {
        functionalPrm.globalParamsEnabled = true;
        init();
    };
    Lung(ExternalParameters extern_prm) : externPrm(extern_prm) {
        init();
    };

    // The following two functions will initialize a new branch and store it in
    // the branches vector of this class
    inline Branch* addBranch(BranchParameters* _branch_prm, Branch* _connection)
    {
        if (functionalPrm.globalParamsEnabled)
            return addBranch(_branch_prm, _connection, &branchPrmGlobal);
        else
            std::cout << "ERROR: Global params not defined! Please use the other add_branch overload." << std::endl;
        exit(1);
        return nullptr;
    }

    inline Branch* addBranch(BranchParameters* _branch_prm, std::vector<Branch*> connections)
    {
        if (functionalPrm.globalParamsEnabled)
            return addBranch(_branch_prm, connections, &branchPrmGlobal);
        else
            std::cout << "ERROR: Global params not defined! Please use the other add_branch overload." << std::endl;
        exit(1);
        return nullptr;
    };

    inline Branch* addBranch(BranchParameters* _branch_prm, Branch* _connection, GlobalBranchParameters* _branch_prm_global)
    {
        // Create new branch with params
        Branch newBranch(_branch_prm_global, *_branch_prm);
        // Add branch into the list of branches and initialize it
        branches.push_back(newBranch);
        branches.back().addConnection(_connection);

        // Adjustments to the lung parameters with the new branch
        lungPrm.addBranchAdjustments(branches.back()._getBranchParameters());

        return &branches.back();
    }

    inline Branch* addBranch(BranchParameters* _branch_prm, std::vector<Branch*> connections, GlobalBranchParameters* _branch_prm_global)
    {
        // Create new branch with params
        Branch newBranch(_branch_prm_global, *_branch_prm);
        // Add branch into the list of branches and initialize it
        branches.push_back(newBranch);
        branches.back().addConnections(connections);

        // Adjustments to the lung parameters with the new branch
        lungPrm.addBranchAdjustments(branches.back()._getBranchParameters());

        return &branches.back();
    };

    void removeBranch(Branch* _branch)
    {
        for (int i = 0; i < branches.size(); i++)
        {
            // Remove all connections to the branch that is getting deleted
            for (int j = 0; j < branches[i].getConnections().size(); j++)
            {
                if (branches[i].getConnections()[j] == _branch) branches[i].deleteConnection(_branch);
            }

            lungPrm.removeBranchAdjustments(_branch->_getBranchParams());

            // Finally remove the branch from the list of branches
            if (&branches[i] == _branch) branches.erase(branches.begin() + i);
        }
    }

    // virtual public member functions
    virtual inline void timeStep(TimeStepParameters* q) = 0;

    virtual void readStructureFromFile(std::ifstream* i) {
        std::cout << "Structure reader has to be implemented into the derived classes in order to use them.";
    };
};
