// -*- mode: c++; c-basic-offset: 2; -*-

/**
 * @file   numa.hh
 * @date   octobre  1, 2023
 * @brief  NUMA utils
 */

#pragma once

#include "numa.h"
#include <unistd.h>

static inline int numa_node_of_page(void *page) {
  int result;
  const auto err =
      numa_move_pages(getpid(), 1, (void **)&page, nullptr, &result, 0);

  return result;
}

static inline void move_region_to_node(int node, void *start, size_t size,
                                       size_t page_size = 4096) {
  const auto page_cnt = size / page_size;
  int *nodes = new int[page_cnt];
  int *status = new int[page_cnt];
  void **pages = new void *[page_cnt];
  for (auto i = 0UL; i < page_cnt; i++) {
    nodes[i] = node;
    status[i] = 1000;
    pages[i] = (void *)((char *)start + i * page_size);
  }

  const auto err = numa_move_pages(getpid(), page_cnt, pages, nodes, status, 0);

  if (err != 0) {
    std::cerr << "Warning: first page might not be on the target node. ";
    std::cerr << "Expected: " << node << ", got: " << status[0] << std::endl;
  }

  delete[] nodes;
  delete[] status;
  delete[] pages;
}
