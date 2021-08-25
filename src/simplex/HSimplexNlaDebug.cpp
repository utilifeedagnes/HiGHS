/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2021 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/*    Authors: Julian Hall, Ivet Galabova, Qi Huangfu, Leona Gottwald    */
/*    and Michael Feldmeier                                              */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/HSimplexNlaDebug.cpp
 *
 * @brief Debugging code for simplex NLA
 */
#include "simplex/HSimplexNlaDebug.h"

#include "util/HighsRandom.h"

//#include <stdio.h>

const double kResidualLargeError = 1e-12;
const double kResidualExcessiveError = sqrt(kResidualLargeError);

const double kSolveLargeError = 1e-12;
const double kSolveExcessiveError = sqrt(kSolveLargeError);

const double kInverseLargeError = 1e-12;
const double kInverseExcessiveError = sqrt(kInverseLargeError);

HighsDebugStatus debugCheckInvert(const HSimplexNla& simplex_nla,
                                  const HighsInt alt_debug_level) {
  // Sometimes a value other than highs_debug_level is passed as
  // alt_debug_level, either to force debugging, or to limit
  // debugging. If no value is passed, then alt_debug_level = -1, and
  // highs_debug_level is used
  HighsInt use_debug_level = alt_debug_level;
  if (use_debug_level<0) use_debug_level = simplex_nla.options_->highs_debug_level;
  if (use_debug_level < kHighsDebugLevelCostly)
    return HighsDebugStatus::kNotChecked;
  // If highs_debug_level isn't being used, then indicate that it's
  // being forced, and also force reporting of OK errors
  const bool force = alt_debug_level > simplex_nla.options_->highs_debug_level;
  if (force) highsLogDev(simplex_nla.options_->log_options, HighsLogType::kInfo,
			 "CheckNlaINVERT:   Forcing debug\n");

  HighsDebugStatus return_status = HighsDebugStatus::kNotChecked;
  return_status = HighsDebugStatus::kOk;

  const HighsInt num_row = simplex_nla.lp_->num_row_;
  const HighsInt num_col = simplex_nla.lp_->num_col_;
  const vector<HighsInt>& a_matrix_start = simplex_nla.lp_->a_matrix_.start_;
  const vector<HighsInt>& a_matrix_index = simplex_nla.lp_->a_matrix_.index_;
  const vector<double>& a_matrix_value = simplex_nla.lp_->a_matrix_.value_;
  const HighsInt* base_index = simplex_nla.base_index_;
  const HighsOptions* options = simplex_nla.options_;
  HVector column;
  HVector rhs;
  column.setup(num_row);
  rhs.setup(num_row);
  double expected_density = 1;

  // Solve for a random solution
  HighsRandom random;
  // Solve Bx=b
  column.clear();
  rhs.clear();
  column.count = -1;
  for (HighsInt iRow = 0; iRow < num_row; iRow++) {
    rhs.index[rhs.count++] = iRow;
    double value = random.fraction();
    column.array[iRow] = value;
    HighsInt iCol = base_index[iRow];
    if (iCol < num_col) {
      for (HighsInt iEl = a_matrix_start[iCol]; iEl < a_matrix_start[iCol + 1];
           iEl++) {
        HighsInt index = a_matrix_index[iEl];
        rhs.array[index] += value * a_matrix_value[iEl];
      }
    } else {
      HighsInt index = iCol - num_col;
      assert(index < num_row);
      rhs.array[index] += value;
    }
  }
  HVector residual = rhs;
  simplex_nla.ftran(rhs, expected_density);

  return_status =
      debugReportError(simplex_nla, false, column, rhs, residual, force);

  // Solve B^Tx=b
  rhs.clear();
  for (HighsInt iRow = 0; iRow < num_row; iRow++) {
    rhs.index[rhs.count++] = iRow;
    HighsInt iCol = base_index[iRow];
    if (iCol < num_col) {
      for (HighsInt iEl = a_matrix_start[iCol]; iEl < a_matrix_start[iCol + 1];
           iEl++) {
        rhs.array[iRow] +=
            column.array[a_matrix_index[iEl]] * a_matrix_value[iEl];
      }
    } else {
      rhs.array[iRow] += column.array[iCol - num_col];
    }
  }
  residual = rhs;
  simplex_nla.btran(rhs, expected_density);

  return_status =
      debugReportError(simplex_nla, true, column, rhs, residual, force);

  if (use_debug_level < kHighsDebugLevelExpensive) return return_status;

  std::string value_adjective;
  HighsLogType report_level;
  expected_density = 0;
  double inverse_error_norm = 0;
  double residual_error_norm = 0;
  for (HighsInt iRow = 0; iRow < num_row; iRow++) {
    HighsInt iCol = base_index[iRow];
    column.clear();
    column.packFlag = true;
    if (iCol < num_col) {
      for (HighsInt k = a_matrix_start[iCol]; k < a_matrix_start[iCol + 1];
           k++) {
        HighsInt index = a_matrix_index[k];
        column.array[index] = a_matrix_value[k];
        column.index[column.count++] = index;
      }
    } else {
      HighsInt index = iCol - num_col;
      column.array[index] = 1.0;
      column.index[column.count++] = index;
    }

    HVector residual = column;
    simplex_nla.ftran(column, expected_density);
    double inverse_column_error_norm = 0;
    for (HighsInt lc_iRow = 0; lc_iRow < num_row; lc_iRow++) {
      double value = column.array[lc_iRow];
      double ckValue;
      if (lc_iRow == iRow) {
        ckValue = 1;
      } else {
        ckValue = 0;
      }
      double inverse_error = fabs(value - ckValue);
      inverse_column_error_norm =
          std::max(inverse_error, inverse_column_error_norm);
    }
    inverse_error_norm =
        std::max(inverse_column_error_norm, inverse_error_norm);
    double residual_column_error_norm =
        debugResidualError(simplex_nla, false, column, residual);
    residual_error_norm =
        std::max(residual_column_error_norm, residual_error_norm);
  }
  if (inverse_error_norm) {
    if (inverse_error_norm > kInverseExcessiveError) {
      value_adjective = "Excessive";
      report_level = HighsLogType::kError;
    } else if (inverse_error_norm > kInverseLargeError) {
      value_adjective = "Large";
      report_level = HighsLogType::kWarning;
    } else {
      value_adjective = "Small";
    }
    highsLogDev(options->log_options, report_level,
                "CheckINVERT:   %-9s (%9.4g) norm for inverse error\n",
                value_adjective.c_str(), inverse_error_norm);
  }

  if (residual_error_norm) {
    if (residual_error_norm > kResidualExcessiveError) {
      value_adjective = "Excessive";
      report_level = HighsLogType::kError;
      return_status = HighsDebugStatus::kError;
    } else if (residual_error_norm > kResidualLargeError) {
      value_adjective = "Large";
      report_level = HighsLogType::kWarning;
      return_status = HighsDebugStatus::kWarning;
    } else {
      value_adjective = "Small";
    }
    if (force) report_level = HighsLogType::kInfo;
    highsLogDev(options->log_options, report_level,
                "CheckINVERT:   %-9s (%9.4g) norm for inverse residual error\n",
                value_adjective.c_str(), residual_error_norm);
  }

  return return_status;
}

double debugResidualError(const HSimplexNla& simplex_nla, const bool transposed,
                          const HVector& solution, HVector& residual) {
  const HighsInt num_row = simplex_nla.lp_->num_row_;
  const HighsInt num_col = simplex_nla.lp_->num_col_;
  const vector<HighsInt>& a_matrix_start = simplex_nla.lp_->a_matrix_.start_;
  const vector<HighsInt>& a_matrix_index = simplex_nla.lp_->a_matrix_.index_;
  const vector<double>& a_matrix_value = simplex_nla.lp_->a_matrix_.value_;
  const HighsInt* base_index = simplex_nla.base_index_;

  if (transposed) {
    for (HighsInt iRow = 0; iRow < num_row; iRow++) {
      HighsInt iCol = base_index[iRow];
      if (iCol < num_col) {
        for (HighsInt iEl = a_matrix_start[iCol];
             iEl < a_matrix_start[iCol + 1]; iEl++) {
          HighsInt index = a_matrix_index[iEl];
          double value = solution.array[index];
          residual.array[iRow] -= value * a_matrix_value[iEl];
        }
      } else {
        HighsInt index = iCol - num_col;
        double value = solution.array[index];
        residual.array[iRow] -= value;
      }
    }
  } else {
    for (HighsInt iRow = 0; iRow < num_row; iRow++) {
      double value = solution.array[iRow];
      HighsInt iCol = base_index[iRow];
      if (iCol < num_col) {
        for (HighsInt iEl = a_matrix_start[iCol];
             iEl < a_matrix_start[iCol + 1]; iEl++) {
          HighsInt index = a_matrix_index[iEl];
          residual.array[index] -= value * a_matrix_value[iEl];
        }
      } else {
        HighsInt index = iCol - num_col;
        residual.array[index] -= value;
      }
    }
  }

  double residual_error_norm = 0;
  for (HighsInt iRow = 0; iRow < num_row; iRow++) {
    double residual_error = fabs(residual.array[iRow]);
    residual_error_norm = std::max(residual_error, residual_error_norm);
  }
  return residual_error_norm;
}

HighsDebugStatus debugReportError(const HSimplexNla& simplex_nla,
                                  const bool transposed,
                                  const HVector& true_solution,
                                  const HVector& solution, HVector& residual,
                                  const bool force) {
  const HighsInt num_row = simplex_nla.lp_->num_row_;
  const HighsOptions* options = simplex_nla.options_;
  double solve_error_norm = 0;
  for (HighsInt iRow = 0; iRow < num_row; iRow++) {
    double solve_error = fabs(solution.array[iRow] - true_solution.array[iRow]);
    solve_error_norm = std::max(solve_error, solve_error_norm);
  }
  double residual_error_norm =
      debugResidualError(simplex_nla, transposed, solution, residual);

  std::string value_adjective;
  HighsLogType report_level;
  HighsDebugStatus return_status = HighsDebugStatus::kOk;

  std::string type = "";
  if (transposed) type = "transposed ";
  if (solve_error_norm) {
    if (solve_error_norm > kSolveExcessiveError) {
      value_adjective = "Excessive";
      report_level = HighsLogType::kError;
    } else if (solve_error_norm > kSolveLargeError) {
      value_adjective = "Large";
      report_level = HighsLogType::kWarning;
    } else {
      value_adjective = "Small";
    }
    if (force) report_level = HighsLogType::kInfo;
    highsLogDev(
        options->log_options, report_level,
        "CheckINVERT:   %-9s (%9.4g) norm for %srandom solution solve error\n",
        value_adjective.c_str(), solve_error_norm, type.c_str());
  }

  if (residual_error_norm) {
    if (residual_error_norm > kResidualExcessiveError) {
      value_adjective = "Excessive";
      report_level = HighsLogType::kError;
      return_status = HighsDebugStatus::kError;
    } else if (residual_error_norm > kResidualLargeError) {
      value_adjective = "Large";
      report_level = HighsLogType::kWarning;
      return_status = HighsDebugStatus::kWarning;
    } else {
      value_adjective = "Small";
    }
    if (force) report_level = HighsLogType::kInfo;
    highsLogDev(options->log_options, report_level,
                "CheckINVERT:   %-9s (%9.4g) norm for %srandom solution "
                "residual error\n",
                value_adjective.c_str(), residual_error_norm, type.c_str());
  }
  return return_status;
}
