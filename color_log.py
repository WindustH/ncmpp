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
    def log(msg, level="INFO"):
        """Log message with color coding matching ncmpp format."""
        color_code = {
            "INFO": '',  # Default foreground color
            "WARN": ColorLogger.YELLOW,
            "ERROR": ColorLogger.RED,
            "DEBUG": ColorLogger.CYAN,
            "SUCCESS": ColorLogger.GREEN
        }.get(level, '')

        print(f"{color_code}[{level}] {msg}{ColorLogger.RESET}")

    @staticmethod
    def info(msg):
        """Log info message."""
        ColorLogger.log(msg, "INFO")

    @staticmethod
    def warn(msg):
        """Log warning message."""
        ColorLogger.log(msg, "WARN")

    @staticmethod
    def error(msg):
        """Log error message."""
        ColorLogger.log(msg, "ERROR")

    @staticmethod
    def debug(msg):
        """Log debug message."""
        ColorLogger.log(msg, "DEBUG")

    @staticmethod
    def success(msg):
        """Log success message."""
        ColorLogger.log(msg, "SUCCESS")

    @staticmethod
    def path(path_str):
        """Format a path in blue color."""
        return f"{ColorLogger.BLUE}{path_str}{ColorLogger.RESET}"

# Global instance for easy import
logger = ColorLogger()
