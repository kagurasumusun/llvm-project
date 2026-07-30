# ci/workflows

The real source of the GitHub Actions workflows.

A GitHub App without the `workflows` permission cannot create or update
anything under `.github/workflows/`. Every route is refused -- `git push`, the
Contents API and the Git Data API all return

    refusing to allow a GitHub App to create or update workflow
    `.github/workflows/<name>.yml` without `workflows` permission

Everything else in the tree is writable. So the workflows are edited here and
`.github/workflows/sync-workflows.yml` mirrors them across on every push that
touches this directory. That job runs with the repository's own `GITHUB_TOKEN`
and is therefore not subject to the restriction.

`sync-workflows.yml` deliberately refuses to update itself, so a change here
cannot rewrite the syncing rules. To change the sync job, edit
`.github/workflows/sync-workflows.yml` by hand.
