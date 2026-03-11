export interface Camera {
  id: string;
  name: string;
  connected: boolean;
  buffer_size: number;
  last_event_time?: string;
  paused?: boolean;
}

export interface CameraStatus {
  cameras: Camera[];
}
