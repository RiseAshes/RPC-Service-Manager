-- 创建数据库
CREATE DATABASE IF NOT EXISTS rpc_service_manager
    DEFAULT CHARACTER SET utf8mb4
    DEFAULT COLLATE utf8mb4_unicode_ci;

USE rpc_service_manager;

-- 服务注册表
CREATE TABLE IF NOT EXISTS services (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    service_name VARCHAR(255) NOT NULL,
    host VARCHAR(255) NOT NULL,
    port INT UNSIGNED NOT NULL,
    metadata JSON,
    is_active TINYINT(1) DEFAULT 1,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    UNIQUE KEY uk_service_endpoint (service_name, host, port),
    INDEX idx_service_name (service_name),
    INDEX idx_is_active (is_active)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 服务心跳记录表
CREATE TABLE IF NOT EXISTS heartbeats (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    service_id BIGINT NOT NULL,
    heartbeat_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (service_id) REFERENCES services(id) ON DELETE CASCADE,
    INDEX idx_service_time (service_id, heartbeat_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- RPC 调用日志表
CREATE TABLE IF NOT EXISTS call_logs (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    service_name VARCHAR(255) NOT NULL,
    method_name VARCHAR(255) NOT NULL,
    caller_host VARCHAR(255),
    server_host VARCHAR(255),
    status_code INT,
    duration_ms INT,
    error_message TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_service (service_name),
    INDEX idx_time (created_at),
    INDEX idx_status (status_code)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 性能指标表
CREATE TABLE IF NOT EXISTS performance_metrics (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    service_name VARCHAR(255) NOT NULL,
    metric_name VARCHAR(255) NOT NULL,
    metric_value DOUBLE,
    metric_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_service_metric (service_name, metric_name),
    INDEX idx_time (metric_time)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
