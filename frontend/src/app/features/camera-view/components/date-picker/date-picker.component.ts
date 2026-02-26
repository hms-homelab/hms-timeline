import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FormsModule } from '@angular/forms';
import { format } from 'date-fns';

/**
 * Date picker component with quick select buttons
 * Allows selecting date for viewing events
 */
@Component({
  selector: 'app-date-picker',
  standalone: true,
  imports: [CommonModule, FormsModule],
  template: `
    <div class="flex items-center gap-3 flex-wrap">
      <!-- Quick Select Buttons -->
      <button
        class="px-4 py-2 rounded-lg text-sm font-medium transition-colors"
        [class.bg-blue-600]="isToday()"
        [class.text-white]="isToday()"
        [class.bg-gray-700]="!isToday()"
        [class.text-gray-300]="!isToday()"
        [class.hover:bg-blue-500]="!isToday()"
        (click)="selectToday()">
        Today
      </button>

      <button
        class="px-4 py-2 bg-gray-700 hover:bg-gray-600 text-gray-300 rounded-lg text-sm font-medium transition-colors"
        (click)="selectYesterday()">
        Yesterday
      </button>

      <!-- Custom Date Input -->
      <input
        type="date"
        [value]="formatDate(selectedDate)"
        (change)="onDateInputChange($event)"
        class="px-4 py-2 bg-gray-700 text-white rounded-lg text-sm border border-gray-600 focus:border-blue-500 focus:outline-none"
        [max]="todayFormatted" />

      <!-- Selected Date Display -->
      <div class="ml-auto text-sm text-gray-400">
        {{ formatDisplayDate(selectedDate) }}
      </div>
    </div>
  `
})
export class DatePickerComponent {
  @Input() selectedDate!: Date;
  @Output() dateChange = new EventEmitter<Date>();

  get todayFormatted(): string {
    return this.formatDate(new Date());
  }

  selectToday() {
    this.dateChange.emit(new Date());
  }

  selectYesterday() {
    const yesterday = new Date();
    yesterday.setDate(yesterday.getDate() - 1);
    this.dateChange.emit(yesterday);
  }

  onDateInputChange(event: Event) {
    const input = event.target as HTMLInputElement;
    const date = new Date(input.value + 'T00:00:00');
    this.dateChange.emit(date);
  }

  isToday(): boolean {
    const today = new Date();
    return this.formatDate(today) === this.formatDate(this.selectedDate);
  }

  formatDate(date: Date): string {
    return format(date, 'yyyy-MM-dd');
  }

  formatDisplayDate(date: Date): string {
    return format(date, 'EEEE, MMMM d, yyyy');
  }
}
