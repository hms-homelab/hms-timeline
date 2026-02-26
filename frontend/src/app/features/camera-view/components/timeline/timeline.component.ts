import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DetectionEvent } from '../../../../core/models/event.model';

/**
 * 24-hour timeline component
 * Displays events as dots on a horizontal timeline
 */
@Component({
  selector: 'app-timeline',
  standalone: true,
  imports: [CommonModule],
  template: `
    <div class="relative">
      <h3 class="text-sm font-medium text-gray-400 mb-3">Timeline</h3>

      <div class="flex overflow-x-auto pb-4">
        @for (hour of hours; track hour) {
          <div class="flex-shrink-0 w-20 border-l border-gray-600 relative">
            <!-- Hour Label -->
            <div class="text-xs text-gray-500 px-2">
              {{ formatHour(hour) }}
            </div>

            <!-- Event Markers -->
            <div class="relative h-12 mt-2">
              @for (event of getEventsForHour(hour); track event.event_id) {
                <button
                  type="button"
                  class="absolute top-1/2 -translate-y-1/2 rounded-full transition-all z-10 cursor-pointer"
                  [class.w-3]="event.event_id !== selectedEventId"
                  [class.h-3]="event.event_id !== selectedEventId"
                  [class.bg-blue-500]="event.event_id !== selectedEventId"
                  [class.hover:scale-150]="event.event_id !== selectedEventId"
                  [class.w-4]="event.event_id === selectedEventId"
                  [class.h-4]="event.event_id === selectedEventId"
                  [class.bg-blue-400]="event.event_id === selectedEventId"
                  [class.ring-2]="event.event_id === selectedEventId"
                  [class.ring-blue-300]="event.event_id === selectedEventId"
                  [class.animate-pulse]="event.event_id === selectedEventId"
                  [style.left.%]="getEventPosition(event)"
                  [title]="getEventTitle(event)"
                  (click)="onEventClick(event)">
                </button>
              }
            </div>
          </div>
        }
      </div>
    </div>
  `,
  styles: [`
    :host {
      display: block;
    }
  `]
})
export class TimelineComponent {
  @Input() events: DetectionEvent[] = [];
  @Input() selectedDate!: Date;
  @Input() selectedEventId: string | null = null;
  @Output() eventSelect = new EventEmitter<DetectionEvent>();

  hours = Array.from({ length: 24 }, (_, i) => i);

  getEventsForHour(hour: number): DetectionEvent[] {
    return this.events.filter(event => {
      const eventDate = new Date(event.started_at);
      return eventDate.getHours() === hour;
    });
  }

  getEventPosition(event: DetectionEvent): number {
    const eventDate = new Date(event.started_at);
    const minutes = eventDate.getMinutes();
    return (minutes / 60) * 100; // Percentage within hour
  }

  getEventTitle(event: DetectionEvent): string {
    const time = new Date(event.started_at).toLocaleTimeString();
    return `${time} - ${event.detected_classes || 'No detections'}`;
  }

  formatHour(hour: number): string {
    return `${hour.toString().padStart(2, '0')}:00`;
  }

  onEventClick(event: DetectionEvent) {
    this.eventSelect.emit(event);
  }
}
