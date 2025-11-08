#!/usr/bin/env python3
"""
Discord Webhook Sender
DevLog를 Discord로 전송하고 피드백을 유도합니다.
"""

import argparse
import json
import re
from pathlib import Path
import urllib.request
import urllib.error
from datetime import datetime

def send_webhook(webhook_url, payload):
    """Discord Webhook으로 메시지 전송"""
    if not webhook_url:
        print("⚠️ Discord webhook URL이 설정되지 않았습니다.")
        return False

    try:
        data = json.dumps(payload).encode('utf-8')
        req = urllib.request.Request(
            webhook_url,
            data=data,
            headers={'Content-Type': 'application/json'}
        )

        with urllib.request.urlopen(req) as response:
            if response.status == 204:
                print("✅ Discord 메시지 전송 성공")
                return True
            else:
                print(f"⚠️ 예상치 못한 응답: {response.status}")
                return False

    except urllib.error.HTTPError as e:
        print(f"❌ HTTP Error: {e.code} - {e.reason}")
        print(f"   Response: {e.read().decode('utf-8')}")
        return False
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def extract_summary_from_daily(md_path):
    """Daily DevLog에서 요약 정보 추출"""
    content = Path(md_path).read_text(encoding='utf-8')

    summary = {}

    # 커밋 수 추출
    match = re.search(r'총 커밋:\s*(\d+)', content)
    if match:
        summary['commits'] = int(match.group(1))

    # 변경 라인 추출
    match = re.search(r'변경 라인:\s*\+(\d+)\s*/\s*-(\d+)', content)
    if match:
        summary['added'] = int(match.group(1))
        summary['deleted'] = int(match.group(2))

    # 핵심 변경 추출 (최대 3개)
    top_changes = []
    in_top_changes = False
    for line in content.split('\n'):
        if '## 1. 오늘의 핵심 변경' in line:
            in_top_changes = True
            continue
        if in_top_changes:
            if line.startswith('##'):
                break
            if line.startswith('- ['):
                # [type] summary 형식 파싱
                match = re.match(r'-\s*\[([^\]]+)\]\s*(.+)', line)
                if match:
                    top_changes.append({
                        'type': match.group(1),
                        'summary': match.group(2).split('—')[0].strip()
                    })
                    if len(top_changes) >= 3:
                        break

    summary['top_changes'] = top_changes

    return summary

def extract_summary_from_weekly(md_path):
    """Weekly DevLog에서 요약 정보 추출"""
    content = Path(md_path).read_text(encoding='utf-8')

    summary = {}

    # 주간 요약 추출
    match = re.search(r'\*\*주간 요약\*\*:\s*(.+)', content)
    if match:
        summary['summary'] = match.group(1).strip()

    # 총 커밋 추출
    match = re.search(r'\*\*총 커밋\*\*:\s*(\d+)개', content)
    if match:
        summary['commits'] = int(match.group(1))

    # 활동 일수 추출
    match = re.search(r'\*\*활동 일수\*\*:\s*(\d+)/7일', content)
    if match:
        summary['active_days'] = int(match.group(1))

    # 신규 기능 수 추출
    features = len(re.findall(r'###\s*신규 기능.*?\n\n(.*?)(?=###|\n##|\Z)', content, re.DOTALL))
    summary['features'] = features

    # 버그 수정 수 추출
    fixes = len(re.findall(r'###\s*버그 수정.*?\n\n(.*?)(?=###|\n##|\Z)', content, re.DOTALL))
    summary['fixes'] = fixes

    return summary

def create_daily_embed(date, summary, devlog_url):
    """Daily DevLog용 Discord Embed 생성"""
    color = 0x5865F2  # Discord 블루

    # 핵심 변경 필드
    changes_text = ""
    if summary.get('top_changes'):
        for change in summary['top_changes'][:3]:
            emoji_map = {
                'feat': '✨',
                'fix': '🐛',
                'refactor': '♻️',
                'perf': '⚡',
                'docs': '📝',
                'test': '✅',
                'chore': '🔧'
            }
            emoji = emoji_map.get(change['type'], '📌')
            changes_text += f"{emoji} **{change['type']}**: {change['summary']}\n"

    if not changes_text:
        changes_text = "변경 사항 없음"

    # 통계 필드
    stats_text = f"📊 커밋: {summary.get('commits', 0)}개\n"
    stats_text += f"➕ 추가: {summary.get('added', 0)} 라인\n"
    stats_text += f"➖ 삭제: {summary.get('deleted', 0)} 라인"

    embed = {
        "title": f"📅 Daily DevLog — {date}",
        "description": "오늘의 개발 활동이 기록되었습니다.",
        "color": color,
        "fields": [
            {
                "name": "🎯 주요 변경 사항",
                "value": changes_text,
                "inline": False
            },
            {
                "name": "📈 통계",
                "value": stats_text,
                "inline": False
            }
        ],
        "footer": {
            "text": "💡 DevLog를 확인하고 피드백을 남겨주세요!"
        },
        "timestamp": datetime.utcnow().isoformat()
    }

    if devlog_url:
        embed["url"] = devlog_url

    return embed

def create_weekly_embed(week_label, summary, devlog_url):
    """Weekly DevLog용 Discord Embed 생성"""
    color = 0xFEE75C  # Discord 옐로우

    # 주간 요약
    summary_text = summary.get('summary', '활동 없음')

    # 통계
    stats_text = f"📊 총 커밋: {summary.get('commits', 0)}개\n"
    stats_text += f"📅 활동 일수: {summary.get('active_days', 0)}/7일\n"
    stats_text += f"✨ 신규 기능: {summary.get('features', 0)}개\n"
    stats_text += f"🐛 버그 수정: {summary.get('fixes', 0)}개"

    embed = {
        "title": f"📊 Weekly Report — {week_label}",
        "description": summary_text,
        "color": color,
        "fields": [
            {
                "name": "📈 주간 통계",
                "value": stats_text,
                "inline": False
            },
            {
                "name": "💭 회고 시간!",
                "value": "Weekly Report에 포함된 회고 질문에 답변해주세요.\n팀원들과 함께 이번 주를 돌아보고 다음 주를 계획해봅시다! 🚀",
                "inline": False
            }
        ],
        "footer": {
            "text": "✍️ 회고 질문에 답변을 작성하고 함께 성장하세요!"
        },
        "timestamp": datetime.utcnow().isoformat()
    }

    if devlog_url:
        embed["url"] = devlog_url

    return embed

def create_feedback_thread(webhook_url, message_content):
    """피드백 스레드 메시지 생성"""
    # Discord에서는 webhook으로 직접 스레드를 만들 수 없으므로
    # 메시지에 피드백 가이드를 포함
    feedback_guide = {
        "content": message_content,
        "components": []  # Discord 봇이 필요한 부분
    }
    return feedback_guide

def main():
    ap = argparse.ArgumentParser(description="Discord Webhook Sender")
    ap.add_argument("--webhook-url", required=True, help="Discord Webhook URL")
    ap.add_argument("--type", choices=["daily", "weekly"], required=True, help="리포트 타입")
    ap.add_argument("--devlog-file", required=True, help="DevLog 파일 경로")
    ap.add_argument("--date", help="날짜 또는 주차 (YYYY-MM-DD 또는 YYYY-WXX)")
    ap.add_argument("--devlog-url", help="DevLog 온라인 URL")
    args = ap.parse_args()

    # DevLog 파일 확인
    devlog_path = Path(args.devlog_file)
    if not devlog_path.exists():
        print(f"❌ DevLog 파일을 찾을 수 없습니다: {args.devlog_file}")
        return 1

    # 요약 정보 추출
    if args.type == "daily":
        summary = extract_summary_from_daily(devlog_path)
        embed = create_daily_embed(args.date, summary, args.devlog_url)
        username = "DevLog Bot 📅"
    else:  # weekly
        summary = extract_summary_from_weekly(devlog_path)
        embed = create_weekly_embed(args.date, summary, args.devlog_url)
        username = "Weekly Report Bot 📊"

    # Webhook 페이로드 구성
    payload = {
        "username": username,
        "embeds": [embed]
    }

    # 전송
    success = send_webhook(args.webhook_url, payload)

    return 0 if success else 1

if __name__ == "__main__":
    exit(main())
