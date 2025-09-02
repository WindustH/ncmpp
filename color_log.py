#!/usr/bin/env python3
"""
color_log.py - Colorful logging utility matching ncmpp format

Provides color-coded logging that matches the C++ ncmpp program format:
- [INFO]    - Green
- [WARN]    - Yellow  
- [ERROR]   - Red
- [DEBUG]   - Cyan
- [SUCCESS] - Green
"""

import sys
from pathlib import Path

class ColorLogger:
    """Color logger that matches ncmpp C++ format."""
    
    # ANSI color codes matching ncmpp
    GREEN = '\033[32m'
    YELLOW = '\033[33m'
    RED = '\033[31m'
    CYAN = '\033[36m'
    BLUE = '\033[34m'
    RESET = '\033[0m'
    
    @staticmethod
    def log(message, level="INFO"):
        """Log message with color coding matching ncmpp format."""
        color_code = {
            "INFO": '',  # Default foreground color
            "WARN": ColorLogger.YELLOW,
            "ERROR": ColorLogger.RED,
            "DEBUG": ColorLogger.CYAN,
            "SUCCESS": ColorLogger.GREEN
        }.get(level, '')
        
        print(f"{color_code}[{level}] {message}{ColorLogger.RESET}")
    
    @staticmethod
    def info(message):
        """Log info message."""
        ColorLogger.log(message, "INFO")
    
    @staticmethod
    def warn(message):
        """Log warning message."""
        ColorLogger.log(message, "WARN")
    
    @staticmethod
    def error(message):
        """Log error message."""
        ColorLogger.log(message, "ERROR")
    
    @staticmethod
    def debug(message):
        """Log debug message."""
        ColorLogger.log(message, "DEBUG")
    
    @staticmethod
    def success(message):
        """Log success message."""
        ColorLogger.log(message, "SUCCESS")
    
    @staticmethod
    def path(path_str):
        """Format a path in blue color."""
        return f"{ColorLogger.BLUE}{path_str}{ColorLogger.RESET}"

# Global instance for easy import
logger = ColorLogger()