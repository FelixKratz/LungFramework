//
//  develop.h
//  OpenLung
//
//  Created by Felix Kratz on 08.07.19.
//  Copyright © 2019 Felix Kratz. All rights reserved.
//

#pragma once
#include <iostream>
#include "branchScaleModel.h"
#include "lungScaleModel.h"

namespace Example {
  void example() {
      // All of the values provided here are arbitrary examples
      double V_ip_init = 1;
      double sigma_0 = 100;
      double tau_ip = 1;
      double dt = 1e-3;

      // Define the needed parameters, these can differ for different models
      GlobalBranchParameters global_branch_prm({.zeta = 1e5});
      ExternalParameters extern_prm({.dt = dt, .P_init = 0., .P_ip_init = -0.5, .V_ip_init = V_ip_init, .N_branches = 2, .Omega = 1});

      // Set up the specific lung that you want to simulate with the previously defined parameters
      BinaryTreeLung binaryTreeLung(global_branch_prm, extern_prm);
      // Enable multithreading for lung models that support asynchronous updates to their branches
      binaryTreeLung.useThreading(4);
      // Enable history tracking, with parameters (trackingRate, detailedHistory). Turning detailed history on will create a deep copy of EVERY lung
      // branch parameter and save it to memory, so be carefull with big networks or many time steps to not run into memory problems.
      // The non-detailed history will save a deep copy of a lung paramter set to memory and is thus much smaller in size.
      binaryTreeLung.trackHistory(1, false);

      // Setup of the branches
      BranchParameters mainBranch({.R = 0, .L = 0, .T = 0, .V = static_cast<float>(150.), .P_th = 0 , .isStatic = true, .layer_ID = 0});
      std::vector<BinaryTreeLungBranch*> connections;
      // This adds the branch to the lung
      connections.push_back(binaryTreeLung.addBranch(&mainBranch, connections));

      // Setup of the second layer is similar to the main layer, however it will have a connection to the main branch.
      BranchParameters secondLayer({.R = 10. , .L = 10., .T = 0.1, .V = 0, .P_th = .1, .layer_ID = 1});
      binaryTreeLung.addBranch(&secondLayer, connections);

      // This triggers the time step for the lung 100 times with Kelvin-Voigt activated opening.
      for (int i = 0; i < 100; i++) {
        TimeStepParameters q({.V_ip = V_ip_init + sigma_0 *V_ip_init* (1 - exp(- i*dt/tau_ip))});
        binaryTreeLung.timeStep(&q);
      }

      // The history can be accessed by calling the method of the lung
      std::vector<HistoryPair>* _history = binaryTreeLung._getHistory();

      // This is how the history can be accessed
      std::cout << "The current pressure is " << _history->back().first.P << " and the pressure 20 time steps ago was " << _history->at(_history->size() - 1 - 20).first.P << std::endl;
      std::cout << "The volume after the first step is " << _history->front().first.V << " and the current volume is " << _history->back().first.V << std::endl;
  }
}
