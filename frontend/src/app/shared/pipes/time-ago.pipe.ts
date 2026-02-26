import { Pipe, PipeTransform } from '@angular/core';
import { formatDistanceToNow } from 'date-fns';

/**
 * Pipe to format dates as "time ago" (e.g., "5 minutes ago")
 * Uses date-fns for formatting
 */
@Pipe({
  name: 'timeAgo',
  standalone: true
})
export class TimeAgoPipe implements PipeTransform {
  transform(value: string | Date | null | undefined): string {
    if (!value) {
      return '';
    }

    try {
      const date = typeof value === 'string' ? new Date(value) : value;
      return formatDistanceToNow(date, { addSuffix: true });
    } catch (error) {
      return '';
    }
  }
}
