Developed at Princeton University

Author: Felix Kratz (felix.kratz@tu-dortmund.de)<br/>
Supervisors: Jean-Francois Louf (jlouf@princeton.edu)<br/>
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
             and Sujit Datta (ssdatta@princeton.edu)

# Citation and use
If you use this work in your research department or company and
you publish data obtained with this, please cite the following
article:

* Following soon

# Use of the Framework
The supplied version of the framework also comes with an example for the
implementation of specific models. The model used as an example is a simple
binary branched lung network with elasto-capillary opening dynamics and
Kelvin-Voigt muscle coupling, as described in detail in the paper above.

## Prerequisites:
This code can be compiled using the **clang** compiler, as it
contains ***Objective-C*** code, by using the provided **Makefile**
with the following commands:
```bash
cd path/to/project/folder
make
```
the **make** command then executes the **Makefile** and creates the
compiled executable in the **./out** directory and executes it.

# Implementing a new model
First, every model needs branch scale and lung scale variables and parameters.
Those parameters can be defined and implemented in the file **./model/model_params.h**, as
currently shown for the binary tree lung model. Those parameters will then be
accessible in the model implementation discussed now.

The framework is grouped into two important categories: branch scale and lung scale behaviour.

First, the branch scale model can be implemented in the **./model/branchScaleModel.h**
file by modifying the function
```C++
inline void timeStep(ExternalParameters* _extern_params,
                     LungParameters* _lung_params,
                     TimeStepParameters* q) override {
    ...
}
```
to fit the desired model. In the branch scale **timeStep** function
the parameters defined previously are accessible, modifiable and can be used to
calculate the branch scale update accordingly.
The **timeStep** function is called for ***every*** individual component and
can access the surrounding components via its **_connections** vector, that
holds pointers to all the connected components, and thus gives access to their
parameters as well, should they be required to calculate the update.

Second, the lung scale model can be implemented in the **./model/lungScaleModel.h**
file, analogously to the branch scale model, by modifying the function
```C++
inline void timeStep(TimeStepParameters* q) override {
    ...
}
```
where the **TimeStepParameters** object is also defined in **./model/model_params.h**
and can hold arbitrary information necessary for the lung scale update.

Note, that the update of the branch scale model is triggered in the
update of the lung scale model, either by using the built-in **updateBranches(q)** member function,
or by using a custom update routine.

If the built-in history tracking is used to monitor and save the time steps
the **triggerHistoryEvent()** function needs to be called in the lung scale
**timeStep** function.

# Creating a new lung geometry
If a model is employed, a lung geometry can be created, as it is demonstrated in
the file **model/example.h**.
To create the geometry simply create a lung object, the one defined in **./model/lungScaleModel.h**, as an example the **BinaryTreeLung** is used:
```C++
BinaryTreeLung binaryTreeLung(global_branch_prm, extern_prm);
```
where ***global_branch_prm*** and ***extern_prm*** are objects of types
**GlobalBranchParameters** and **ExternalParameters** defined in **./model/model_params.h** that hold the lung scale information needed for the simulation.

Next, a branch can be added to the lung, by calling the **addBranch** member function
of the lung:
```C++
binaryTreeLung.addBranch(&branch_prm, connections);
```
where ***branch_prm*** is an object of type **BranchParameters** that
holds all specific informations about the branch (e.g. radius, length, etc.)
and ***connections*** is a vector of pointers of other branches, to which
the branch should be connected:
```C++
std::vector<BinaryTreeLungBranch*> connections;
```

With the provided framework arbitrary lung geometries can be implemented, with
arbitrarily complicated connections between them and arbitrarily complex time step
updates. The provided example is only a very simple demonstration. In the
above paper a drastically bigger lung geometry is explored, still yielding
great performance and most important, yielding good agreement with clinic studies.

# Performing the simulation
If everything is set up, the simulation can be performed with a simple loop,
like it is shown in the example **model/example.h**
```C++
for (int i = 0; i < steps; i++) {
  TimeStepParameters q( ... )
  binaryTreeLung.timeStep(&q);
}
```
where ***steps*** many time steps are performed with **TimeStepParameters** that
can be individual for each time step.

Finally, in the **./main.cpp** call the simulation function and use the
**make** command in a command line that has its working directory set to the
folder where the **main.cpp** and the **Makefile** reside, as it is shown in the
***Prerequisites*** section of this document.
