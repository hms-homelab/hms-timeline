import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionEvent } from '../../../../core/models/event.model';
import { TimeAgoPipe } from '../../../../shared/pipes/time-ago.pipe';
import { DurationPipe } from '../../../../shared/pipes/duration.pipe';
import { LazyLoadDirective } from '../../../../shared/directives/lazy-load.directive';
import { toRelativeMediaUrl } from '../../../../core/utils/media-url';

/**
 * Event card component with snapshot on the right
 * Displays event metadata, play button, and snapshot thumbnail
 */
@Component({
  selector: 'app-event-card',
  standalone: true,
  imports: [CommonModule, TimeAgoPipe, DurationPipe, LazyLoadDirective],
  template: `
    <div
      class="event-card w-full text-left flex items-center gap-4"
      [class.selected]="isSelected"
      [class.cursor-pointer]="hasDetections()"
      [class.opacity-50]="!hasDetections()"
      (click)="onPlayClick()">

      <!-- Event Info (Left Side) -->
      <div class="event-info flex-1">
        <!-- Timestamp and Duration with Play Icon -->
        <div class="flex items-center justify-between text-sm">
          <div class="flex items-center gap-2">
            @if (isSelected && hasDetections()) {
              <!-- Playing indicator -->
              <svg class="w-5 h-5 text-blue-400 flex-shrink-0 animate-pulse" fill="currentColor" viewBox="0 0 20 20">
                <path d="M10 18a8 8 0 100-16 8 8 0 000 16zM9.555 7.168A1 1 0 008 8v4a1 1 0 001.555.832l3-2a1 1 0 000-1.664l-3-2z"/>
              </svg>
              <div class="flex flex-col">
                <span class="text-blue-400 font-medium">Now Playing</span>
                <span class="text-xs text-blue-300">{{ getExactTime() }}</span>
              </div>
            } @else if (hasDetections() && event.recording_url) {
              <!-- Play icon (only for events with detections) -->
              <svg class="w-5 h-5 text-blue-500 flex-shrink-0" fill="currentColor" viewBox="0 0 20 20">
                <path d="M6.3 2.841A1.5 1.5 0 004 4.11V15.89a1.5 1.5 0 002.3 1.269l9.344-5.89a1.5 1.5 0 000-2.538L6.3 2.84z"/>
              </svg>
              <div class="flex flex-col">
                <span class="text-gray-300 font-medium">{{ event.started_at | timeAgo }}</span>
                <span class="text-xs text-gray-500">{{ getExactTime() }}</span>
              </div>
            } @else {
              <!-- No detections — motion-only indicator -->
              <svg class="w-5 h-5 text-gray-600 flex-shrink-0" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M15 12a3 3 0 11-6 0 3 3 0 016 0z" />
                <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M2.458 12C3.732 7.943 7.523 5 12 5c4.478 0 8.268 2.943 9.542 7-1.274 4.057-5.064 7-9.542 7-4.477 0-8.268-2.943-9.542-7z" />
              </svg>
              <div class="flex flex-col">
                <span class="text-gray-500 font-medium">{{ event.started_at | timeAgo }}</span>
                <span class="text-xs text-gray-600">{{ getExactTime() }} - motion only</span>
              </div>
            }
          </div>
          @if (event.duration_seconds) {
            <span class="text-gray-400">{{ event.duration_seconds | duration }}</span>
          }
        </div>

        <!-- Detected Objects Badges -->
        @if (event.detected_classes) {
          <div class="detections mt-2">
            @for (cls of getDetectedClasses(); track cls) {
              <span class="badge">{{ cls }}</span>
            }
          </div>
        }

        <!-- AI Context -->
        @if (event.ai_context) {
          <p class="text-sm text-gray-300 italic mt-2">
            "{{ event.ai_context }}"
          </p>
        }

        <!-- Detection Count and Camera -->
        <div class="flex items-center justify-between mt-2 text-xs text-gray-500">
          <span>{{ event.camera_name }}</span>
          @if (event.total_detections > 0) {
            <span>
              {{ event.total_detections }} detection{{ event.total_detections !== 1 ? 's' : '' }}
            </span>
          } @else {
            <span class="text-gray-600">no detections</span>
          }
        </div>
      </div>

      <!-- Snapshot Thumbnail (Right Side) -->
      <div class="flex-shrink-0 w-32 h-24 bg-gray-900 rounded-lg overflow-hidden border"
           [class.border-gray-700]="hasDetections()"
           [class.border-gray-800]="!hasDetections()">
        @if (hasSnapshot()) {
          <img
            [appLazyLoad]="getSnapshotUrl()"
            [alt]="'Detection at ' + event.started_at"
            class="w-full h-full object-cover" />
        } @else {
          <div class="w-full h-full flex items-center justify-center text-gray-700">
            <svg class="w-8 h-8" fill="none" viewBox="0 0 24 24" stroke="currentColor">
              <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z" />
            </svg>
          </div>
        }
      </div>
    </div>
  `
})
export class EventCardComponent {
  @Input() event!: DetectionEvent;
  @Input() isSelected: boolean = false;
  @Output() play = new EventEmitter<void>();

  hasDetections(): boolean {
    return this.event.total_detections > 0;
  }

  hasSnapshot(): boolean {
    const url = this.event.snapshot_url;
    return !!url && url !== '' && url !== 'http://192.168.2.5:8000/snapshots/None';
  }

  onPlayClick() {
    if (this.hasDetections() && this.event.recording_url) {
      this.play.emit();
    }
  }

  getDetectedClasses(): string[] {
    if (!this.event.detected_classes) {
      return [];
    }
    return this.event.detected_classes.split(', ').filter(c => c.trim());
  }

  getSnapshotUrl(): string {
    return toRelativeMediaUrl(this.event.snapshot_url, 'snapshots');
  }

  getExactTime(): string {
    // Format exact time (e.g., "2:45 PM" or "14:45")
    const date = new Date(this.event.started_at);
    return date.toLocaleTimeString('en-US', {
      hour: 'numeric',
      minute: '2-digit',
      hour12: true
    });
  }
}
