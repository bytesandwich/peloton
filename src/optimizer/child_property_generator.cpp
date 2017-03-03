//===----------------------------------------------------------------------===//
//
//                         Peloton
//
// child_property_generator.cpp
//
// Identification: src/include/optimizer/child_property_generator.cpp
//
// Copyright (c) 2015-16, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "optimizer/child_property_generator.h"
#include "optimizer/column_manager.h"
#include "optimizer/properties.h"

namespace peloton {
namespace optimizer {

std::vector<std::pair<PropertySet, std::vector<PropertySet>>>
ChildPropertyGenerator::GetProperties(std::shared_ptr<GroupExpression> gexpr,
                                      PropertySet requirements) {
  requirements_ = requirements;
  output_.clear();

  gexpr->Op().Accept(this);

  return std::move(output_);
}

void ChildPropertyGenerator::Visit(const PhysicalScan *) {
  PropertySet provided_property;
  /*for (auto &property : requirements_.Properties()) {
    if (property->Type() == PropertyType::PREDICATE ||
        property->Type() == PropertyType::COLUMNS) {
      provided_property.AddProperty(property);
    }
  }*/
  //LOG_DEBUG("Deriving Child Properties");
  auto predicate_prop =
      requirements_.GetPropertyOfType(PropertyType::PREDICATE);

  if (predicate_prop != nullptr) {
    provided_property.AddProperty(predicate_prop);
  }

  auto columns_prop = requirements_.GetPropertyOfType(PropertyType::COLUMNS)
                          ->As<PropertyColumns>();

  if (columns_prop != nullptr) {
    std::vector<expression::TupleValueExpression *> column_exprs;
    for (size_t i = 0; i < columns_prop->GetSize(); ++i)
      column_exprs.push_back(columns_prop->GetColumn(i));

    // Add sort column to output property if needed
    auto sort_prop =
        requirements_.GetPropertyOfType(PropertyType::SORT)->As<PropertySort>();
    // column_exprs.empty() if 
    // the sql is 'SELECT *'
    if (sort_prop != nullptr && !column_exprs.empty()) {
      for (size_t i = 0; i < sort_prop->GetSortColumnSize(); ++i) {
        auto sort_col = sort_prop->GetSortColumn(i);
        bool found = false;
        if (columns_prop != nullptr) {
          for (auto &col : column_exprs) {
            if (std::get<2>(sort_col->bound_obj_id) ==
                std::get<2>(col->bound_obj_id)) {
              found = true;
              break;
            }
          }
        }

        if (!found) column_exprs.push_back(sort_col);
      }
    }
    //for (auto &column_expr : column_exprs) {
    //  oid_t id = std::get<2>(column_expr->bound_obj_id);
    //  LOG_DEBUG("output column %u", id);
    //}

    provided_property.AddProperty(
        std::shared_ptr<PropertyColumns>(new PropertyColumns(column_exprs)));
  }
  output_.push_back(
      std::make_pair(std::move(provided_property), std::vector<PropertySet>()));
};

void ChildPropertyGenerator::Visit(const PhysicalProject *){};
void ChildPropertyGenerator::Visit(const PhysicalOrderBy *) {}
void ChildPropertyGenerator::Visit(const PhysicalFilter *){};
void ChildPropertyGenerator::Visit(const PhysicalInnerNLJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalLeftNLJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalRightNLJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalOuterNLJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalInnerHashJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalLeftHashJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalRightHashJoin *){};
void ChildPropertyGenerator::Visit(const PhysicalOuterHashJoin *){};

} /* namespace optimizer */
} /* namespace peloton */
