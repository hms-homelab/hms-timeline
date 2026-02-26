import { Component, Input } from '@angular/core';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-loading-spinner',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="flex items-center justify-center p-8">
      <div class="loading-spinner"></div>
      @if (message) {
        <span class="ml-3 text-ring-light-gray">{{ message }}</span>
      }
    </div>
  `
})
export class LoadingSpinnerComponent {
  @Input() message?: string;
}
