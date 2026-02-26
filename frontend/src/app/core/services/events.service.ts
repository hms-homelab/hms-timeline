import { Injectable, inject } from '@angular/core';
import { Observable } from 'rxjs';
import { ApiService } from './api.service';
import {
  DetectionEvent,
  EventsResponse,
  EventDetail,
  TimelineData
} from '../models/event.model';

/**
 * Service for detection events operations
 * Queries event data, timeline, and event details from database
 */
@Injectable({
  providedIn: 'root'
})
export class EventsService {
  private api = inject(ApiService);

  /**
   * Get events with optional filters
   * @param cameraId Optional camera filter
   * @param startDate Optional start date
   * @param endDate Optional end date
   * @param limit Maximum number of events (default 100)
   */
  getEvents(
    cameraId?: string,
    startDate?: Date,
    endDate?: Date,
    limit: number = 100
  ): Observable<DetectionEvent[]> {
    const params: Record<string, any> = { limit };

    if (cameraId) {
      params['camera_id'] = cameraId;
    }

    if (startDate) {
      params['start'] = startDate.toISOString();
    }

    if (endDate) {
      params['end'] = endDate.toISOString();
    }

    return this.api.get<EventsResponse>('api/events', params).pipe(
      map(response => response.events)
    );
  }

  /**
   * Get events for a specific camera and date (convenience method)
   * @param cameraId Camera identifier
   * @param date Date to query
   */
  getEventsByDate(cameraId: string, date: Date): Observable<DetectionEvent[]> {
    const startDate = new Date(date);
    startDate.setHours(0, 0, 0, 0);

    const endDate = new Date(date);
    endDate.setHours(23, 59, 59, 999);

    return this.getEvents(cameraId, startDate, endDate);
  }

  /**
   * Get detailed event information including all detections
   * @param eventId Event identifier
   */
  getEventDetail(eventId: string): Observable<EventDetail> {
    return this.api.get<EventDetail>(`api/events/${eventId}`);
  }

  /**
   * Get timeline data for a specific camera and date
   * Returns hourly aggregated event counts
   * @param cameraId Camera identifier
   * @param date Date to query
   */
  getTimelineData(cameraId: string, date: Date): Observable<TimelineData> {
    const dateStr = date.toISOString().split('T')[0]; // YYYY-MM-DD format
    return this.api.get<TimelineData>('api/timeline', {
      camera_id: cameraId,
      date: dateStr
    });
  }
}

// Import map operator
import { map } from 'rxjs/operators';
