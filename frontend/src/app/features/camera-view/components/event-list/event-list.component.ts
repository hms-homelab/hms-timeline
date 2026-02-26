import { Component, Input, Output, EventEmitter } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ScrollingModule } from '@angular/cdk/scrolling';
import { DetectionEvent } from '../../../../core/models/event.model';
import { EventCardComponent } from '../event-card/event-card.component';

/**
 * Event list component with virtual scrolling
 * Displays list of detection events as cards
 */
@Component({
  selector: 'app-event-list',
  standalone: true,
  imports: [CommonModule, ScrollingModule, EventCardComponent],
  template: `
    <div class="space-y-4">
      @for (event of events; track event.event_id) {
        <app-event-card
          [event]="event"
          [isSelected]="event.event_id === selectedEventId"
          (play)="onEventPlay(event)">
        </app-event-card>
      }
    </div>
  `
})
export class EventListComponent {
  @Input() events: DetectionEvent[] = [];
  @Input() selectedEventId: string | null = null;
  @Output() eventPlay = new EventEmitter<DetectionEvent>();

  onEventPlay(event: DetectionEvent) {
    this.eventPlay.emit(event);
  }
}
