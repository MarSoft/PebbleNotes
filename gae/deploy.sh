#!/bin/bash
set -e

ENV=${1:-prod}

case "$ENV" in
    prod|production)
        ENV="prod"
        PROJECT="pebble-notes"
        ;;
    staging)
        PROJECT="pebble-notes-staging"
        ;;
    *)
        echo "Usage: $0 [prod|staging]"
        exit 1
        ;;
esac

SECRETS_FILE="secret-${ENV}.env"

if [ ! -f "$SECRETS_FILE" ]; then
    echo "Error: $SECRETS_FILE not found"
    echo "Create it with OAUTH_CLIENT_ID and OAUTH_CLIENT_SECRET"
    exit 1
fi

# Load secrets
source "$SECRETS_FILE"

if [ -z "$OAUTH_CLIENT_ID" ] || [ -z "$OAUTH_CLIENT_SECRET" ]; then
    echo "Error: OAUTH_CLIENT_ID and OAUTH_CLIENT_SECRET must be set in $SECRETS_FILE"
    exit 1
fi

echo "Deploying to $PROJECT..."

# Create temporary app.yaml with secrets
cat app.yaml > app-deploy.yaml
cat >> app-deploy.yaml <<EOF

env_variables:
  OAUTH_CLIENT_ID: "${OAUTH_CLIENT_ID}"
  OAUTH_CLIENT_SECRET: "${OAUTH_CLIENT_SECRET}"
EOF

# Deploy
gcloud app deploy app-deploy.yaml --project="$PROJECT" --quiet

# Cleanup
rm app-deploy.yaml

echo "Deployed to https://${PROJECT}.appspot.com"
