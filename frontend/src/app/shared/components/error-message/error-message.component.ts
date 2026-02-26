import { Component, Input } from '@angular/core';
import { CommonModule } from '@angular/common';

@Component({
  selector: 'app-error-message',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="p-4 bg-ring-red/10 border border-ring-red/30 rounded-lg">
      <div class="flex items-start">
        <svg class="w-6 h-6 text-ring-red flex-shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M12 8v4m0 4h.01M21 12a9 9 0 11-18 0 9 9 0 0118 0z" />
        </svg>
        <div class="ml-3 flex-1">
          <h3 class="text-sm font-medium text-ring-red">{{ title || 'Error' }}</h3>
          @if (message) {
            <p class="mt-1 text-sm text-ring-light-gray">{{ message }}</p>
          }
        </div>
      </div>
    </div>
  `
})
export class ErrorMessageComponent {
  @Input() title?: string;
  @Input() message?: string;
}
