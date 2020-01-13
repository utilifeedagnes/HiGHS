/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2020 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/HighsSimplexAnalysis.h
 * @brief Analyse simplex iterations, both for run-time control and data gathering
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#ifndef SIMPLEX_HIGHSSIMPLEXANALYSIS_H_
#define SIMPLEX_HIGHSSIMPLEXANALYSIS_H_

#include "lp_data/HighsLp.h"
#include "lp_data/HighsOptions.h"
#include "simplex/SimplexConst.h"
#include "simplex/HVector.h"
#include "util/HighsTimer.h"
#include "util/HighsUtils.h"

#ifdef HiGHSDEV
  enum ANALYSIS_OPERATION_TYPE {
    ANALYSIS_OPERATION_TYPE_BTRAN_FULL = 0,
    ANALYSIS_OPERATION_TYPE_PRICE_FULL,
    ANALYSIS_OPERATION_TYPE_BTRAN_EP,
    ANALYSIS_OPERATION_TYPE_PRICE_AP,
    ANALYSIS_OPERATION_TYPE_FTRAN,
    ANALYSIS_OPERATION_TYPE_FTRAN_BFRT,
    ANALYSIS_OPERATION_TYPE_FTRAN_DSE,
    NUM_ANALYSIS_OPERATION_TYPE,
  };
#endif
  const double running_average_multiplier = 0.05;

/**
 * @brief Analyse simplex iterations, both for run-time control and data gathering
 */
class HighsSimplexAnalysis {
 public:
  void setup(const HighsLp& lp, const HighsOptions& options, const int simplex_iteration_count_);
  void messaging(FILE* logfile_, FILE* output_, const int message_level_);
  void updateOperationResultDensity(const double local_density, double& density);
  void iterationReport();
  void invertReport();
  void invertReport(const bool header);
  void dualSteepestEdgeWeightError(const double computed_edge_weight, const double updated_edge_weight);
  bool switchToDevex();

#ifdef HiGHSDEV
  HighsTimer timer_;
  void iterationRecord();
  void iterationRecordMajor();
  void operationRecordBefore(const int operation_type, const HVector& vector, const double historical_density);
  void operationRecordBefore(const int operation_type, const int current_count, const double historical_density);
  void operationRecordAfter(const int operation_type, const HVector& vector);
  void operationRecordAfter(const int operation_type, const int result_count);
  void summaryReport();
#endif

  int numRow;
  int numCol;
  int numTot;
  bool allow_dual_steepest_edge_to_devex_switch;
  double dual_steepest_edge_weight_log_error_threshhold;
  FILE* logfile;
  FILE* output;
  int message_level;

  double col_aq_density;
  double row_ep_density;
  double row_ap_density;
  double row_DSE_density;
  
  int simplex_strategy = 0;
  int min_threads = 0;
  int num_threads = 0;
  int max_threads = 0;
  int multi_num = 0;
  DualEdgeWeightMode edge_weight_mode = DualEdgeWeightMode::STEEPEST_EDGE;
  int solve_phase = 0;
  int simplex_iteration_count = 0;
  int multi_iteration_count = 0;
  int devex_iteration_count = 0;
  int multi_chosen = 0;
  int multi_finished = 0;
  int pivotal_row_index = 0;
  int leaving_variable = 0;
  int entering_variable = 0;
  int num_primal_infeasibilities = 0;
  int num_dual_infeasibilities = 0;
  int invert_hint = 0;
  int freelist_size = 0;
  double reduced_rhs_value = 0;
  double reduced_cost_value = 0;
  double edge_weight = 0;
  double primal_delta = 0;
  double primal_step = 0;
  double dual_step = 0;
  double pivot_value_from_column = 0;
  double pivot_value_from_row = 0;
  double numerical_trouble = 0;
  double objective_value = 0;
  double sum_primal_infeasibilities = 0;
  double sum_dual_infeasibilities = 0;
  double basis_condition = 0;
  int num_devex_framework = 0;

  int num_col_price = 0;
  int num_row_price = 0;
  int num_row_price_with_switch = 0;

#ifdef HiGHSDEV
  HighsValueDistribution before_ftran_upper_sparse_density;
  HighsValueDistribution ftran_upper_sparse_density;
  HighsValueDistribution before_ftran_upper_hyper_density;
  HighsValueDistribution ftran_upper_hyper_density;
  HighsValueDistribution cost_perturbation1_distribution;
  HighsValueDistribution cost_perturbation2_distribution;
  HighsValueDistribution cleanup_dual_change_distribution;
  HighsValueDistribution cleanup_primal_step_distribution;
  HighsValueDistribution cleanup_dual_step_distribution;
  HighsValueDistribution cleanup_primal_change_distribution;
#endif

 private:

  void iterationReport(const bool header);
  void reportAlgorithmPhaseIterationObjective(const bool header, const int this_message_level);
  void reportInfeasibility(const bool header, const int this_message_level);
#ifdef HiGHSDEV
  void reportThreads(const bool header, const int this_message_level);
  void reportMulti(const bool header, const int this_message_level);
  void reportOneDensity(const int this_message_level, const double density);
  void reportOneDensity(const double density);
  void reportDensity(const bool header, const int this_message_level);
  void reportInvert(const bool header, const int this_message_level);
  void reportCondition(const bool header, const int this_message_level);
  void reportIterationData(const bool header, const int this_message_level);
  void reportFreeListSize(const bool header, const int this_message_level);
  int intLog10(const double v);
#endif
  bool dualAlgorithm();

  int AnIterNumCostlyDseIt;  //!< Number of iterations when DSE is costly
  double AnIterCostlyDseFq;  //!< Frequency of iterations when DSE is costly
  const double AnIterCostlyDseMeasureLimit = 1000.0;  //!<
  const double AnIterCostlyDseMnDensity = 0.01;       //!<
  const double AnIterFracNumTot_ItBfSw = 0.1;         //!<
  const double AnIterFracNumCostlyDseItbfSw = 0.05;   //!<
  double AnIterCostlyDseMeasure;
  int AnIterPrevRpNumCostlyDseIt;  //!< Number of costly DSE iterations when
                                   //!< previously reported


  const double accept_weight_threshhold = 0.25;
  const double weight_error_threshhold = 4.0;

  int num_dual_steepest_edge_weight_check = 0;
  int num_dual_steepest_edge_weight_reject = 0;
  int num_wrong_low_dual_steepest_edge_weight = 0;
  int num_wrong_high_dual_steepest_edge_weight = 0;
  double average_frequency_low_dual_steepest_edge_weight = 0;
  double average_frequency_high_dual_steepest_edge_weight = 0;
  double average_log_low_dual_steepest_edge_weight_error = 0;
  double average_log_high_dual_steepest_edge_weight_error = 0;
  double max_average_frequency_low_dual_steepest_edge_weight = 0;
  double max_average_frequency_high_dual_steepest_edge_weight = 0;
  double max_sum_average_frequency_extreme_dual_steepest_edge_weight = 0;
  double max_average_log_low_dual_steepest_edge_weight_error = 0;
  double max_average_log_high_dual_steepest_edge_weight_error = 0;
  double max_sum_average_log_extreme_dual_steepest_edge_weight_error = 0;

  const int iteration_report_message_level = ML_DETAILED;
  const int invert_report_message_level = ML_MINIMAL;
  int num_invert_report_since_last_header = -1;
  int num_iteration_report_since_last_header = -1;

  double average_num_threads;
  double average_fraction_of_possible_minor_iterations_performed;
  int sum_multi_chosen = 0;
  int sum_multi_finished = 0;

  int AnIterIt0 = 0;
#ifdef HiGHSDEV
  int AnIterPrevIt;
  // Major operation analysis struct
  struct AnIterOpRec {
    double AnIterOpLog10RsDensity;
    double AnIterOpSuLog10RsDensity;
    double AnIterOpHyperCANCEL;
    double AnIterOpHyperTRAN;
    int AnIterOpRsDim;
    int AnIterOpNumCa;
    int AnIterOpNumHyperOp;
    int AnIterOpNumHyperRs;
    int AnIterOpRsMxNNZ;
    int AnIterOpSuNumCa;
    int AnIterOpSuNumHyperOp;
    int AnIterOpSuNumHyperRs;
    std::string AnIterOpName;
    HighsValueDistribution AnIterOp_density;
  };
  AnIterOpRec AnIterOp[NUM_ANALYSIS_OPERATION_TYPE];

  struct AnIterTraceRec {
    double AnIterTraceTime;
    double AnIterTraceMulti;
    double AnIterTraceDensity[NUM_ANALYSIS_OPERATION_TYPE];
    double AnIterTraceCostlyDse;
    int AnIterTraceIter;
    int AnIterTrace_dual_edge_weight_mode;
  };

  enum AnIterTraceMxNumRec { AN_ITER_TRACE_MX_NUM_REC = 20 };
  enum DUAL_EDGE_WEIGHT_MODE_COUNT { DUAL_EDGE_WEIGHT_MODE_COUNT = 3 };
  int AnIterTraceNumRec;
  int AnIterTraceIterDl;
  AnIterTraceRec AnIterTrace[1 + AN_ITER_TRACE_MX_NUM_REC + 1];

  int AnIterNumInvert[INVERT_HINT_Count];
  int AnIterNumEdWtIt[(int)DualEdgeWeightMode::Count];

  HighsValueDistribution primal_step_distribution;
  HighsValueDistribution dual_step_distribution;
  HighsValueDistribution pivot_distribution;
  HighsValueDistribution numerical_trouble_distribution;
#endif
};

#endif /* SIMPLEX_HIGHSSIMPLEXANALYSIS_H_ */
