#pragma once

#include "ISUD_Base.h"

#include <ilcplex/ilocplex.h>


class IB_ComplementaryProblem {
private:
	// Pointer on the problem
	ISUD_Base* psolutionMethod_;
	// Current phase of the CP
	int currentPhase_;
	// Vector of normalization constraint coefficients
	std::vector<double> normalizationConstraint_;
	
	// CPLEX variables
	IloEnv env;
	IloModel mod;
	IloNumVarArray vars;
	IloRangeArray constraints, activeConstraints;
	std::vector<IloRange> artificialConstraints;

	// Vars identifiers
	std::map<int, int> varsIds;

	// Constraints identifiers and columns original indices
	std::map<std::string, int> constraintsIds;
	std::vector<int> colsIndices;

	// Artificial column id
	int acolId_ = -1;
	// Artificial column support
	std::set<int> acSupport_;

	// Objective of CPLEX
	IloObjective cost;

	// Number of columns aggregated in artificial column
	int cardSupport_;

	// Max penalization of artificial column
	double maxPenalization = 0;

	// Artificial column variable id
	int varsacolId = 0;

	// CPLEX solver
	IloCplex main_cplex;

	//verbose level
	// 0 no print
	// 1 basic info
	// 2 detailed info
	//
	int verbose_level = 1;
	
public:
	// Constructor of complementary problem
	// "psolutionMethod" is a pointer on the problem
	// "currentPhase" is the current phase of CP
	// "acolId" is the identifier of the artificial column id
	// "acSupport" is the support of the artificial columns
	// "cardSupport" is the cardinal of "acSupport"
	IB_ComplementaryProblem(ISUD_Base* psolutionMethod, int currentPhase = -1, int acolId = -1, std::vector<int>* acSupport = NULL, int cardSupport = 0) : psolutionMethod_(psolutionMethod),
		currentPhase_(currentPhase), mod(env),  vars(env), constraints(env, psolutionMethod_->tasks_.size() + 1, 0, 0), acolId_(acolId),
		cardSupport_(cardSupport), activeConstraints(env), main_cplex(mod)
		{
		if (acSupport != NULL) {
			for (auto columnId : *acSupport) {
				IB_Column* column = psolutionMethod_->columns_[columnId];
				if (column->isInCurrentSolution() || (currentPhase_ == -1 || column->getPhase() <= currentPhase_)) {
					acSupport_.insert(columnId);
				}
			}
		}

		for (int i = 0; i < psolutionMethod_->columns_.size(); i++) {
			if (i == acolId_) {
				normalizationConstraint_.push_back(1);
			}
			else {
				normalizationConstraint_.push_back(psolutionMethod_->columns_[i]->isInCurrentSolution() ? 1:(psolutionMethod_->columns_[i]->getPhase() + 1));
			}
			
		}
		cost = IloAdd(mod, IloMinimize(env));
	}

	// Construct the complementary problem
	// "increaseArtificialCost" is a boolean that is true if we want to increase the artificial column cost
	// "penalization" is the penalization of the artificialcolumn.
	void constructProblem(bool increaseArtificialCost = false, double penalization = 0);

	// Set phase of the complementary problem
	// "phase" is the CP phase
	void setPhase(int phase) {
		currentPhase_ = phase;
	}


	// Solve the complementary problem
	// Return solution in the vector "solution"
	// Past solution can be NULL
	// Return dual solution in "duals"
	double solve(std::vector<double>* solution, std::vector<double>* pastSolution = NULL, std::vector<double>* duals=NULL);
	
	// Destroy the complementary problem
	void destroy();
};