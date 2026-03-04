export interface DetectionEvent {
  event_id: string;
  camera_id: string;
  camera_name: string;
  started_at: string;
  ended_at?: string;
  duration_seconds?: number;
  total_detections: number;
  status: string;
  recording_url?: string;
  snapshot_url?: string;
  detected_classes?: string;
  max_confidence?: number;
  ai_context?: string;
}

export interface EventsResponse {
  events: DetectionEvent[];
  count: number;
}

export interface Detection {
  detection_id?: number;
  class_name: string;
  confidence: number;
  bbox_x1: number;
  bbox_y1: number;
  bbox_x2: number;
  bbox_y2: number;
  frame_number?: number;
  detected_at?: string;
}

export interface EventDetail {
  event: DetectionEvent;
  detections: Detection[];
}

export interface TimelineHour {
  hour: number;
  event_count: number;
  total_detections: number;
}

export interface TimelineData {
  camera_id: string;
  date: string;
  hours: TimelineHour[];
}

export interface PeriodicSnapshot {
  type: 'snapshot';
  snapshot_id: number;
  camera_id: string;
  camera_name: string;
  captured_at: string;
  snapshot_url: string;
  thumbnail_url?: string;
  ai_context?: string;
}

export interface SearchResult {
  type: 'event' | 'snapshot';
  id: string;
  camera_id: string;
  camera_name: string;
  timestamp: string;
  recording_url?: string;
  snapshot_url?: string;
  total_detections: number;
  duration_seconds?: number;
  detected_classes?: string;
  ai_context?: string;
  rank?: number;
  similarity?: number;
}

export interface SearchResponse {
  events: SearchResult[];
  count: number;
  search_mode: 'fts' | 'semantic';
  query: string;
}
