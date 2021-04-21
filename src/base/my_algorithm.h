// Copyright (C) 2021 - Sebastien Alaiwan
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

#pragma once

#include "span.h"

namespace my
{
template<typename T, typename U>
void swap(T& a, U& b)
{
  auto tmp = a;
  a = b;
  b = tmp;
}

template<typename T, typename Lambda>
void heapify(Span<T> arr, int heapSize, int rootIndex, Lambda less)
{
  const int left = 2 * rootIndex + 1;
  const int right = 2 * rootIndex + 2;

  int largest = rootIndex;

  if(left < heapSize && less(arr[largest], arr[left]))
    largest = left;

  if(right < heapSize && less(arr[largest], arr[right]))
    largest = right;

  if(largest != rootIndex)
  {
    swap(arr[rootIndex], arr[largest]);
    heapify(arr, heapSize, largest, less);
  }
}

template<typename T, typename Lambda>
void sort(Span<T> arr, Lambda less)
{
  auto heapSize = arr.len;

  // Build heap (rearrange array)
  for(int i = heapSize / 2 - 1; i >= 0; i--)
    heapify(arr, heapSize, i, less);

  // One by one extract an element from heap
  for(int i = heapSize - 1; i >= 0; i--)
  {
    // Move current root to end
    swap(arr[0], arr[i]);

    // call max heapify on the reduced heap
    heapify(arr, i, 0, less);
  }
}
}

