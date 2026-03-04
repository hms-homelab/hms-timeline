import { Component, Output, EventEmitter, signal } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';

export interface SearchQuery {
  query: string;
  classes: string[];
}

@Component({
  selector: 'app-search-bar',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <div class="space-y-3">
      <!-- Search Input -->
      <div class="relative">
        <svg class="absolute left-3 top-1/2 -translate-y-1/2 w-5 h-5 text-gray-400" fill="none" viewBox="0 0 24 24" stroke="currentColor">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M21 21l-6-6m2-5a7 7 0 11-14 0 7 7 0 0114 0z" />
        </svg>
        <input
          type="text"
          [(ngModel)]="searchText"
          (ngModelChange)="onSearchChange()"
          (keydown.enter)="onSubmit()"
          placeholder="Search events... (e.g., 'person on porch', 'dog at night')"
          class="w-full pl-10 pr-10 py-2.5 bg-gray-700 border border-gray-600 rounded-lg text-white placeholder-gray-400 focus:outline-none focus:border-blue-500 focus:ring-1 focus:ring-blue-500 text-sm" />
        @if (searchText) {
          <button
            type="button"
            (click)="onClear()"
            class="absolute right-3 top-1/2 -translate-y-1/2 text-gray-400 hover:text-white">
            <svg class="w-5 h-5" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12" />
            </svg>
          </button>
        }
      </div>

      <!-- Class Filter Chips -->
      <div class="flex flex-wrap gap-2">
        @for (cls of availableClasses; track cls) {
          <button
            type="button"
            (click)="toggleClass(cls)"
            class="px-3 py-1 rounded-full text-xs font-medium transition-colors"
            [class.bg-blue-600]="selectedClasses().includes(cls)"
            [class.text-white]="selectedClasses().includes(cls)"
            [class.bg-gray-700]="!selectedClasses().includes(cls)"
            [class.text-gray-300]="!selectedClasses().includes(cls)"
            [class.hover:bg-gray-600]="!selectedClasses().includes(cls)">
            {{ cls }}
          </button>
        }
      </div>

      <!-- Search Status -->
      @if (statusText) {
        <div class="text-xs text-gray-400 flex items-center gap-2">
          @if (isLoading) {
            <svg class="animate-spin h-3.5 w-3.5 text-blue-400" xmlns="http://www.w3.org/2000/svg" fill="none" viewBox="0 0 24 24">
              <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
              <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
            </svg>
            <span>Searching...</span>
          } @else {
            <span>{{ statusText }}</span>
          }
        </div>
      }
    </div>
  `
})
export class SearchBarComponent {
  @Output() search = new EventEmitter<SearchQuery>();
  @Output() clear = new EventEmitter<void>();

  searchText = '';
  statusText = '';
  isLoading = false;
  availableClasses = ['person', 'dog', 'cat', 'car', 'package'];
  selectedClasses = signal<string[]>([]);

  private debounceTimer: any;

  onSearchChange() {
    clearTimeout(this.debounceTimer);
    if (!this.searchText.trim()) {
      this.statusText = '';
      this.clear.emit();
      return;
    }
    this.debounceTimer = setTimeout(() => this.onSubmit(), 300);
  }

  onSubmit() {
    const query = this.searchText.trim();
    if (!query) return;
    this.search.emit({ query, classes: this.selectedClasses() });
  }

  onClear() {
    this.searchText = '';
    this.statusText = '';
    this.selectedClasses.set([]);
    this.clear.emit();
  }

  toggleClass(cls: string) {
    const current = this.selectedClasses();
    if (current.includes(cls)) {
      this.selectedClasses.set(current.filter(c => c !== cls));
    } else {
      this.selectedClasses.set([...current, cls]);
    }
    // Re-trigger search if we have a query
    if (this.searchText.trim()) {
      this.onSubmit();
    }
  }

  setStatus(text: string, loading: boolean = false) {
    this.statusText = text;
    this.isLoading = loading;
  }
}
